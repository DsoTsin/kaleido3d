#ifndef __VkRHI_h__
#define __VkRHI_h__
#pragma once
#include "VkObjects.h"
#include <Core/Thread/Thread.h>
#include <Core/Thread/ConditionVariable.h>
#include <list>
#include <tuple>

namespace k3d
{
	class IShaderCompilerOutput;
}

K3D_VK_BEGIN

class Device;
using RefDevice = std::shared_ptr<Device>;

class ResourceManager;
using PtrResManager = std::unique_ptr<ResourceManager>;

class CommandAllocator;
using PtrCmdAlloc = std::shared_ptr<CommandAllocator>;

class Semaphore;
using PtrSemaphore = std::shared_ptr<Semaphore>;

class Fence;
using PtrFence = std::shared_ptr<Fence>;

class CommandContext;
using PtrContext = std::shared_ptr<CommandContext>;
using PtrContextList = std::list<PtrContext>;

class CommandContextPool;

class SwapChain;
using PtrSwapChain = std::unique_ptr<SwapChain>;

extern K3D_API void EnumAllDeviceAdapter(rhi::IDeviceAdapter** &, uint32*);

struct RHIRoot
{
	using DeviceList	= std::vector<VkPhysicalDevice>;
	struct Initializer
	{
						Initializer(std::string appName, bool enableValidation);
						~Initializer();


		VkInstance		Instance;
		DeviceList		PhysicalDevices;

	private:
		VkResult		Init(bool enableValidation, std::string name);
		VkResult		CreateInstance(bool enableValidation, std::string name);
	};

	static DeviceList&	GetPhysicDevices() { return s_Impl.PhysicalDevices; }
	static VkInstance&	GetInstance() { return s_Impl.Instance; }

private:
	friend class						Device;
	static Initializer					s_Impl;
};

class DeviceAdapter : public rhi::IDeviceAdapter
{
	friend class Device;
public:
	explicit DeviceAdapter(VkPhysicalDevice * pDevice) : m_pPDevice(pDevice) {}

	rhi::IDevice * GetDevice() override;

private:
	VkPhysicalDevice * m_pPDevice;
};

class K3D_API Device : public rhi::IDevice
{
public:
	typedef Device * Ptr;

								Device();
								~Device();
	Result						Create(rhi::IDeviceAdapter *, bool withDebug) override;

	rhi::ICommandContext*		NewCommandContext(rhi::ECommandType)override;
	rhi::IGpuResource*			NewGpuResource(rhi::ResourceDesc const&,uint64)override;
	rhi::ISampler*				NewSampler(const rhi::SamplerState&)override;
	
	rhi::IPipelineStateObject*	NewPipelineState(rhi::EPipelineType)override;
	rhi::IPipelineStateObject* 	CreatePipelineStateObject(rhi::PipelineDesc const & desc);

	rhi::ISyncFence*			NewFence()override;
	rhi::IDescriptorPool *		NewDescriptorPool() override;
	rhi::IRenderViewport *		NewRenderViewport(void * winHandle, uint32 width, uint32 height) override;
	::k3d::IShaderCompiler *	NewShaderCompiler() override;

	VkQueue const&				GetRawDeviceQueue() const { return m_DefaultQueue; }
	VkDevice const&				GetRawDevice() const { return m_Device; }
	VkPhysicalDevice const*		GetRawPhysicDevice() const { return m_pPhysicDevice; }
	PtrResManager const &		GetMemoryManager() const { return m_ResourceManager; }
	PtrCmdAlloc					NewCommandAllocator(bool transient);
	PtrSemaphore				NewSemaphore();


	bool						FindMemoryType(uint32_t typeBits, VkFlags requirementsMask, uint32 *typeIndex) const;

	uint32						GetQueueCount() const { return m_QueueCount; }

private:

	PtrResManager						m_ResourceManager;
	std::unique_ptr<CommandContextPool>	m_ContextPool;

	VkPhysicalDeviceMemoryProperties	m_MemoryProperties;
	VkQueue								m_DefaultQueue;
	VkDevice							m_Device;
	VkPhysicalDevice *					m_pPhysicDevice;
	uint32								m_GfxQueueIndex;
	uint32_t							m_QueueCount;
};

class K3D_API DeviceChild
{
public:
	explicit					DeviceChild(Device *pDevice) : m_pDevice(pDevice) {}
	virtual						~DeviceChild() {}

	VkDevice const &			GetRawDevice() const { return m_pDevice->GetRawDevice(); }
	VkPhysicalDevice const &	GetPhysicalDevice() const { return *(m_pDevice->GetRawPhysicDevice()); }
	VkQueue const &				GetRawQueue() const { return m_pDevice->GetRawDeviceQueue(); }

	Device::Ptr const			GetDevice() const { return m_pDevice; }
private:
	Device::Ptr					m_pDevice;
};

#define DEVICE_CHILD_CONSTRUCT(className) \
	explicit className(Device::Ptr ptr) : DeviceChild(ptr) {}
#define DEVICE_CHILD_CONSTRUCT_DECLARE(className) \
	explicit className(Device::Ptr ptr);

class Fence : public rhi::ISyncFence, public DeviceChild
{
public:
	Fence(Device::Ptr pDevice, VkFenceCreateInfo const & info = FenceCreateInfo::Create())
		: DeviceChild(pDevice)
	{
		K3D_VK_VERIFY(vkCreateFence(GetRawDevice(), &info, nullptr, &m_Fence));
	}

	~Fence() override { vkDestroyFence(GetRawDevice(), m_Fence, nullptr); }

	void Signal(int32 val) {}
	void Reset() override { vkResetFences(GetRawDevice(), 1, &m_Fence); }
	void WaitFor(uint64 time) override 
	{
		vkWaitForFences(GetRawDevice(), 1, &m_Fence, VK_TRUE, time);
	}
private:
	friend class SwapChain;
	friend class CommandContext;

	VkFence m_Fence;
};

class DescriptorAllocator : public rhi::IDescriptorPool, public DeviceChild
{
public:
	explicit DescriptorAllocator(Device::Ptr pDevice);
	~DescriptorAllocator() override;

private:
	VkDescriptorPool m_Pool;
};

class K3D_API Resource : public rhi::IGpuResource, public DeviceChild
{
public:
	typedef void *				Ptr;
	typedef void const*			CPtr;

	explicit					Resource(Device::Ptr pDevice) : DeviceChild(pDevice), m_HostMem(nullptr), m_DeviceMem{} {}
	virtual						~Resource();

	Resource::CPtr				GetHostMemory(uint64 OffSet) const { return m_HostMem; }
	VkDeviceMemory				GetDeviceMemory() const { return m_DeviceMem; }

	Resource::Ptr				Map(uint64 offset, uint64 size) override;
	void						UnMap() override {	vkUnmapMemory(GetRawDevice(), m_DeviceMem); }

protected:
	VkMemoryAllocateInfo		m_MemAllocInfo;
	VkDeviceMemory				m_DeviceMem;
	Resource::Ptr				m_HostMem;

	VkDeviceSize				m_Size = 0;
	VkDeviceSize				m_AllocationSize = 0;
	VkDeviceSize				m_AllocationOffset = 0;
};

class K3D_API Buffer : public Resource
{
public:
	typedef Buffer * Ptr;
	explicit					Buffer(Device::Ptr pDevice) : Resource(pDevice) {}
								Buffer(Device::Ptr pDevice, uint64 size) : Resource(pDevice) { Create(size); }
								~Buffer() override;

	void						Create(size_t size);

private:
	VkDescriptorBufferInfo		m_BufferInfo;
	VkBufferView				m_BufferView;
	VkBuffer					m_Buffer = VK_NULL_HANDLE;
	VkBufferUsageFlags  		m_Usage = 0;
};

class K3D_API Texture : public Resource
{
public:
	typedef Texture * Ptr;
	explicit					Texture(Device::Ptr pDevice) : Resource(pDevice) {}
								~Texture() override;

	const VkImageView &			GetView() const { return m_ImageView; }

	friend class				SwapChain;
private:
	VkImageView					m_ImageView;
	VkImage						m_Image;
};


class K3D_API ResourceManager : public DeviceChild
{
public:

	struct Allocation 
	{
		VkDeviceMemory	Memory = VK_NULL_HANDLE;
		VkDeviceSize	Offset = 0;
		VkDeviceSize	Size = 0;
	};
	
	template <typename VkObjectT>
	struct ResDesc 
	{
		VkObjectT				Object;
		bool					IsTransient = false;
		uint32_t				MemoryTypeIndex;
		VkMemoryPropertyFlags	MemoryProperty;
		VkMemoryRequirements	MemoryRequirements;
	};
	
	template <typename VkObjectT> class PoolManager;

	template <typename VkObject>
	class Pool
	{
	public:
		typedef std::unique_ptr< Pool<VkObject> > PoolPtr;
		virtual ~Pool() {}

		static PoolPtr			Create(VkDevice device, const VkDeviceSize poolSize, const typename ResDesc<VkObject> & objDesc);

		uint32					GetMemoryTypeIndex() const { return m_MemoryTypeIndex; }
		VkDeviceSize			GetSize() const { return m_Size; }
		bool					HasAvailable(VkMemoryRequirements memReqs) const;


		Allocation				Allocate(const typename ResDesc<VkObject> & objDesc);
		friend class			PoolManager<VkObject>;
	private:
		Pool(uint32 memTypeIndex, VkDeviceMemory mem, VkDeviceSize sz);
		
		std::vector<Allocation> m_Allocations;

		uint32					m_MemoryTypeIndex = UINT32_MAX;
		VkDeviceMemory			m_Memory = VK_NULL_HANDLE;
		VkDeviceSize			m_Size = 0;
		VkDeviceSize			m_Offset = 0;
	};


	template <typename VkObjectT>
	class PoolManager : public DeviceChild
	{
	public:
		using PoolRef =					std::unique_ptr< Pool<VkObjectT> >;

										PoolManager(Device::Ptr pDevice, VkDeviceSize poolSize);
		virtual							~PoolManager();
		void							Destroy();
		Allocation						Allocate(const typename ResDesc<VkObjectT>& objDesc);

		VkDeviceSize					GetBlockSize() const { return m_PoolSize; }
	private:
		VkDevice						m_Device = VK_NULL_HANDLE;
		size_t							m_PoolSize = 0;
		std::mutex						m_Mutex;
		std::vector<PoolRef>			m_Pools;
	};

	explicit ResourceManager(Device::Ptr pDevice, size_t bufferBlockSize, size_t imageBlockSize);

	~ResourceManager();

	Allocation  AllocateBuffer(VkBuffer buffer, bool transient, VkMemoryPropertyFlags memoryProperty);
	Allocation	AllocateImage(VkImage image, bool transient, VkMemoryPropertyFlags memoryProperty);

private:
	void		Initialize();
	void		Destroy();

	PoolManager<VkBuffer>				m_BufferAllocations;
	PoolManager<VkImage>				m_ImageAllocations;
};

class CommandContextPool : public DeviceChild
{
public:
	CommandContextPool(Device::Ptr pDevice);
	~CommandContextPool() override;

	CommandContext* RequestContext();

	PtrCmdAlloc	RequestCommandAllocator();

private:
	using Mutex = Concurrency::Mutex;
	Mutex m_PoolMutex;
	Mutex m_ContextMutex;
	std::unordered_map<uint32, PtrCmdAlloc> m_AllocatorPool;
	std::unordered_map<uint32, std::list<CommandContext*> > m_ContextList;
};


/**
 * Need CommandBufferManager to issue Cmds
 */
class K3D_API CommandContext : public rhi::ICommandContext, public DeviceChild
{
public:
	explicit				CommandContext(Device::Ptr pDevice);
							CommandContext(Device::Ptr pDevice, VkCommandBuffer cmdBuf);
	virtual					~CommandContext();

	void					Detach(rhi::IDevice *) override;
	void					CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src) override;

	/**
	 * @brief	submit command buffer to queue and wait for scheduling.
	 * @param	Wait	true to wait.
	 */
	void					Execute(bool Wait) override;
	void					Reset() override;

	void					SubmitAndWait(PtrSemaphore wait, PtrSemaphore signal, PtrFence fence);


	void				Begin();
	void				End();

	void				BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
		uint32 firstSet, uint32 descriptorSetCount, const VkDescriptorSet* pDescriptorSets,
		uint32 dynamicOffsetCount, const uint32* pDynamicOffsets);
	void				BindDescriptorSet(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, const VkDescriptorSet& pDescriptorSets);
	void				ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor,
		uint32 rangeCount, const VkImageSubresourceRange* pRanges);
	void				ClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil,
		uint32 rangeCount, const VkImageSubresourceRange* pRanges);
	void				ClearAttachments(uint32 attachmentCount, const VkClearAttachment* pAttachments, uint32 rectCount, const VkClearRect* pRects);
	void				PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
		VkDependencyFlags dependencyFlags, uint32 memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
		uint32 bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
		uint32 imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
	void				PipelineBarrierBufferMemory(const BufferMemoryBarrierParams& params);
	void				PipelineBarrierImageMemory(const ImageMemoryBarrierParams& params);
	void				PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32 offset, uint32 size, const void* pValues);
	void				BeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);
	void				SetScissorRects(uint32 count, VkRect2D* pRects);
	/**
	*	@param contents VK_SUBPASS_CONTENTS_INLINE,VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
	*/
	void				NextSubpass(VkSubpassContents contents);
	void				EndRenderPass();

	void				ClearColorBuffer(rhi::IColorBuffer* iColorBuffer)override;
	void				ClearDepthBuffer(rhi::IDepthBuffer* iDepthBuffer)override;
	void				SetRenderTargets(uint32 NumColorBuffer, rhi::IColorBuffer *, rhi::IDepthBuffer *, bool ReadOnlyDepth = false) override;
	void				SetViewport(const rhi::ViewportDesc &)override;
	void				SetScissorRects(uint32, const rhi::Rect*)override;
	void				SetIndexBuffer(const rhi::IndexBufferView& IBView) override;
	void				SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView) override;
	void				SetPipelineState(uint32 hashCode, rhi::IPipelineStateObject*)override;
	void				SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout) override;
	void				SetPrimitiveType(rhi::EPrimitiveType)override;
	void				DrawInstanced(rhi::DrawInstanceParam)override;
	void				DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)override;
	void				Dispatch(uint32 X, uint32 Y, uint32 Z) override;

	friend class			CommandContextPool;
protected:
	VkCommandBuffer			m_CommandBuffer;
	VkRenderPass		m_RenderPass;

private:
	void					InitCommandBufferPool();
};

class K3D_API SwapChain : public DeviceChild
{
public:
	DEVICE_CHILD_CONSTRUCT_DECLARE(SwapChain)
		~SwapChain();

	void			Initialize(void* WindowHandle, 
						rhi::EPixelFormat const & PixelFormat,
						uint32 Width, uint32 Height,
						uint32* NumBackBuffers, 
						std::vector<VkImage>& OutImages);

	uint32			GetPresentQueueFamilyIndex() const { return m_SelectedPresentQueueFamilyIndex; }
	uint32			AcquireNextImage(PtrSemaphore presentSemaphore, PtrFence pFence);
	void			Present(uint32 imageIndex, PtrSemaphore renderingFinishSemaphore);
	uint32			GetBackBufferCount() const { return m_ReserveBackBufferCount; }

private:

	VkSurfaceKHR	m_Surface	= VK_NULL_HANDLE;
	VkSwapchainKHR	m_SwapChain = VK_NULL_HANDLE;
	uint32			m_SelectedPresentQueueFamilyIndex;
	uint32			m_ReserveBackBufferCount;

private:

	void InitProcs();

	void Destroy();

	/** private functions */
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
};

class K3D_API RenderViewport : public rhi::IRenderViewport, public DeviceChild
{
public:
						RenderViewport(rhi::IDevice*pDevice, void*windowHandle, uint32 width, uint32 height);
						~RenderViewport() override; 
	bool				InitViewport(void *windowHandle, rhi::IDevice * pDevice,
							uint32 width, uint32 height, rhi::EPixelFormat rtFmt) override;

	void				InitFrameBuffers(VkRenderPass renderPass, VkImageView depthView = VK_NULL_HANDLE);

	/* for test */
	void				RecordCmdBuffers();
	void				PrepareNextFrame() override;

	bool				Present(bool vSync) override;

protected:

	/* for test */
	std::vector<PtrContext>			m_PresentContext;
	/* for test */
	PtrSemaphore					m_PresentSemaphore;
	/* for test */
	PtrSemaphore					m_RenderSemaphore;

	void							InitializePresentContext(uint32 Count);

private:

	uint32							m_CurFrameId;
	PtrSwapChain					m_pSwapChain;
	uint32							m_NumBufferCount;
	std::vector<VkImage>			m_SwapChainImages;
};

class K3D_API PipelineLayout : public rhi::IPipelineLayout
{
public:
	void Create(rhi::ShaderParamLayout const &) override;
	void Finalize(rhi::IDevice *) override;


private:
	VkPipelineLayout m_PipelineLayout;
	VkDescriptorSet * m_pDescriptorSets;
};

class K3D_API PipelineStateObject : public rhi::IPipelineStateObject, public DeviceChild
{
public:
										PipelineStateObject(Device::Ptr pDevice, rhi::PipelineDesc const& desc);
	explicit							PipelineStateObject(Device *pDevice);
	virtual								~PipelineStateObject();

	void								BindRenderPass(VkRenderPass RenderPass);
	void								SetShader(rhi::EShaderType, k3d::IShaderCompilerOutput*) override;
	void								SetLayout(rhi::IPipelineLayout *) override;

	void								SetRasterizerState(const rhi::RasterizerState&)override;
	void								SetBlendState(const rhi::BlendState&)override;
	void								SetDepthStencilState(const rhi::DepthStencilState&)override;
	void								SetSampler(rhi::ISampler*)override;
	void								SetVertexInputLayout(rhi::VertexDeclaration const*, uint32 Count) override;
	void								SetPrimitiveTopology(const rhi::EPrimitiveType) override;
	void								SetRenderTargetFormat(const rhi::RenderTargetFormat &) override;

	VkPipeline 							GetPipeline() const { return m_Pipeline; }
	void								Finalize() override;

	/**
	 * TOFIX
	 */
	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Graphics;
	}

protected:
	void								InitWithDesc(rhi::PipelineDesc const & desc);

	friend class						CommandContext;

	VkPipeline							m_Pipeline;
	VkPipelineCache						m_PipelineCache;
	union 
	{
		VkGraphicsPipelineCreateInfo	m_GfxCreateInfo;
		VkComputePipelineCreateInfo		m_CptCreateInfo;
	};
	VkRenderPass						m_RenderPass;

private:
	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStageInfos;
	VkPipelineInputAssemblyStateCreateInfo	m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo	m_RasterizationState;
	VkPipelineColorBlendStateCreateInfo		m_ColorBlendState;
	VkPipelineDepthStencilStateCreateInfo	m_DepthStencilState;
	VkPipelineViewportStateCreateInfo		m_ViewportState;
	VkPipelineMultisampleStateCreateInfo	m_MultisampleState;
	VkPipelineVertexInputStateCreateInfo	m_VertexInputState;
	std::vector<VkVertexInputBindingDescription>	m_BindingDescriptions;
	std::vector<VkVertexInputAttributeDescription>	m_AttributeDescriptions;
};

class Sampler : public rhi::ISampler, public DeviceChild
{
public:
	explicit	Sampler(Device::Ptr pDevice);
				~Sampler() override;
protected:
	VkSampler m_Sampler;
};

K3D_VK_END

#endif