#ifndef __VkRHI_h__
#define __VkRHI_h__
#pragma once
#include "VkObjects.h"
#include <Core/Os.h>
#include <list>
#include <tuple>

#ifdef __cplusplus
extern  "C" {
#endif
K3D_API void InitializeVulkanRHI(const char* appName, bool debug);
K3D_API void DestroyVulkanRHI();
#ifdef __cplusplus
}
#endif

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
using SwapChainRef = k3d::SharedPtr<SwapChain>;
using PtrSwapChain = std::unique_ptr<SwapChain>;

class CommandQueue;
using SpCmdQueue = std::shared_ptr<CommandQueue>;

class PipelineLayout;
class DescriptorAllocator;
class DescriptorSetLayout;

class FrameBuffer;
using SpFramebuffer = std::shared_ptr<FrameBuffer>;

class RenderPass;
using SpRenderpass = std::shared_ptr<RenderPass>;
using UpRenderpass = std::unique_ptr<RenderPass>;

class Texture;
using SpTexture = std::shared_ptr<Texture>;

class RenderTarget;
using SpRenderTarget = std::shared_ptr<RenderTarget>;

class RenderViewport;
class Sampler;

class ShaderResourceView;
using BindingArray = DynArray<VkDescriptorSetLayoutBinding>;

class DescriptorAllocator;
using DescriptorAllocRef = SharedPtr<DescriptorAllocator>;

class DescriptorSetLayout;
using DescriptorSetLayoutRef = SharedPtr<DescriptorSetLayout>;

class DescriptorSet;
using DescriptorSetRef = SharedPtr<DescriptorSet>;

typedef std::map<rhi::PipelineLayoutKey, rhi::PipelineLayoutRef> MapPipelineLayout;
typedef std::map<uint32, DescriptorSetLayoutRef>			MapDescriptorSetLayout;
typedef std::map<uint32, DescriptorAllocRef>				MapDescriptorAlloc;

struct RHIRoot
{
	static void			Initialize(const char* appName, bool debug);
	static void			Destroy();
	static void			SetupDebug(VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callBack);
	static uint32		GetHostGpuCount();
	static GpuRef		GetHostGpuById(uint32 id);
	static VkInstance	GetInstance();

	static rhi::DeviceRef	GetDeviceById(uint32 id);
	static void				AddViewport(RenderViewport *);
	static RenderViewport *	GetViewport(int index);

private:
	static InstanceRef	s_InstanceRef;
	static RenderViewport* s_Vp;
	
	static void			EnumLayersAndExts();
	friend class		Device;
};

class DeviceAdapter : public rhi::IDeviceAdapter
{
	friend class Device;
public:
	typedef SharedPtr<DeviceAdapter> Ptr;
	explicit DeviceAdapter(GpuRef const& gpu);
	~DeviceAdapter() override;
	rhi::DeviceRef 		GetDevice() override;
private:
	GpuRef				m_Gpu;
	rhi::DeviceRef		m_pDevice;
};

class Device : public rhi::IDevice
{
public:
	typedef Device * Ptr;

								Device();
								~Device() override;
	Result						Create(rhi::IDeviceAdapter *, bool withDebug) override;
	void						Destroy();

	rhi::CommandContextRef		NewCommandContext(rhi::ECommandType)override;
	rhi::GpuResourceRef			NewGpuResource(rhi::ResourceDesc const&)override;
	rhi::ShaderResourceViewRef	NewShaderResourceView(rhi::GpuResourceRef, rhi::ResourceViewDesc const&) override;
	rhi::SamplerRef				NewSampler(const rhi::SamplerState&)override;
	
	rhi::PipelineLayoutRef		NewPipelineLayout(rhi::PipelineLayoutDesc const & table) override;

	DescriptorAllocRef			NewDescriptorAllocator(uint32 maxSets, BindingArray const& bindings);
	DescriptorSetLayoutRef		NewDescriptorSetLayout(BindingArray const& bindings);

	rhi::PipelineStateObjectRef NewPipelineState(rhi::PipelineDesc const & desc, rhi::PipelineLayoutRef ppl,rhi::EPipelineType)override;
	rhi::PipelineStateObjectRef CreatePipelineStateObject(rhi::PipelineDesc const & desc, rhi::PipelineLayoutRef ppl);

	rhi::SyncFenceRef			NewFence()override;
	rhi::IDescriptorPool *		NewDescriptorPool() override;
	rhi::RenderViewportRef		NewRenderViewport(void * winHandle, rhi::GfxSetting&) override;
	void						QueryTextureSubResourceLayout(rhi::GpuResourceRef, rhi::TextureResourceSpec const& spec, rhi::SubResourceLayout *) override;
	SwapChainRef				NewSwapChain(rhi::GfxSetting const& setting);

	SpCmdQueue const&			GetDefaultCmdQueue() const { return m_DefCmdQueue; }

	VkDevice const&				GetRawDevice() const { return m_Device; }
	PtrResManager const &		GetMemoryManager() const { return m_ResourceManager; }

	PtrCmdAlloc					NewCommandAllocator(bool transient);
	bool						FindMemoryType(uint32 typeBits, VkFlags requirementsMask, uint32 *typeIndex) const;
	PtrSemaphore				NewSemaphore();
	void						WaitIdle() override { m_Gpu->vkDeviceWaitIdle(m_Device); }

	uint32						GetQueueCount() const { return m_Gpu->m_QueueProps.Count(); }
	SpRenderpass const &		GetTopPass() const { return m_PendingPass.back(); }
	void						PushRenderPass(SpRenderpass renderPass) { m_PendingPass.push_back(renderPass); }

	rhi::RenderTargetRef		NewRenderTarget(rhi::RenderTargetLayout const & layout) override;

	uint64						GetMaxAllocationCount();

	friend class 				DeviceChild;
protected:
	SpCmdQueue					InitCmdQueue(VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex);

private:
	CmdBufManagerRef						m_CmdBufManager;
	PtrResManager							m_ResourceManager;
	std::unique_ptr<CommandContextPool>		m_ContextPool;
	std::vector<SpRenderpass>				m_PendingPass;

	MapPipelineLayout						m_CachedPipelineLayout;
	MapDescriptorAlloc						m_CachedDescriptorPool;
	MapDescriptorSetLayout					m_CachedDescriptorSetLayout;

private:
	VkPhysicalDeviceMemoryProperties		m_MemoryProperties = {};

	SpCmdQueue								m_DefCmdQueue;
	SpCmdQueue								m_ComputeCmdQueue;

	VkDevice								m_Device = VK_NULL_HANDLE;
	GpuRef									m_Gpu;
};

class DeviceChild
{
public:
	explicit					DeviceChild(Device *pDevice) : m_pDevice(pDevice) {}
	virtual						~DeviceChild() {}

	VkDevice const &			GetRawDevice() const { return m_pDevice->GetRawDevice(); }
	GpuRef						GetGpuRef() const { return m_pDevice->m_Gpu; }

	Device::Ptr const			GetDevice() const { return m_pDevice; }
	SpCmdQueue const&			GetImmCmdQueue() const { return m_pDevice->GetDefaultCmdQueue(); }
	SpCmdQueue const&			GetComputeCmdQueue() const { return m_pDevice->m_ComputeCmdQueue; }
private:
	Device::Ptr					m_pDevice;
};

#define DEVICE_CHILD_CONSTRUCT(className) \
	explicit className(Device::Ptr ptr) : DeviceChild(ptr) {}
#define DEVICE_CHILD_CONSTRUCT_DECLARE(className) \
	explicit className(Device::Ptr ptr = nullptr);

/**
 * Fences are signaled by the system when work invoked by vkQueueSubmit completes.
 */
class Fence : public rhi::ISyncFence, public DeviceChild
{
public:
	Fence(Device::Ptr pDevice, VkFenceCreateInfo const & info = FenceCreateInfo::Create())
		: DeviceChild(pDevice)
	{
		if (pDevice) 
		{
			K3D_VK_VERIFY(GetGpuRef()->vkCreateFence(GetRawDevice(), &info, nullptr, &m_Fence));
		}
	}

	~Fence() override 
	{
		if (m_Fence)
		{
			GetGpuRef()->vkDestroyFence(GetRawDevice(), m_Fence, nullptr);
			VKLOG(Info, "Fence Destroyed. -- %0x.", m_Fence);
			m_Fence = VK_NULL_HANDLE;
		}
	}

	void Signal(int32 val) override {}

	bool IsSignaled()
	{
		return VK_SUCCESS== GetGpuRef()->vkGetFenceStatus(GetRawDevice(), m_Fence);
	}

	void Reset() override { GetGpuRef()->vkResetFences(GetRawDevice(), 1, &m_Fence); }
	void WaitFor(uint64 time) override 
	{
		GetGpuRef()->vkWaitForFences(GetRawDevice(), 1, &m_Fence, VK_TRUE, time);
	}
private:
	friend class SwapChain;
	friend class CommandContext;

	VkFence m_Fence = VK_NULL_HANDLE;
};

/**
 * @param binding 
 */
VkDescriptorSetLayoutBinding RHIBinding2VkBinding(rhi::shc::Binding const & binding);
/**
 * Vulkan has DescriptorPool, DescriptorSet, DescriptorSetLayout and PipelineLayout
 * DescriptorSet is allocated from DescriptorPool with DescriptorSetLayout, 
 * PipelineLayout depends on DescriptorSetLayout,
 */
class DescriptorAllocator : public rhi::IDescriptorPool, public DeviceChild
{
public:

	struct Options
	{
		VkDescriptorPoolCreateFlags			CreateFlags = 0;
		VkDescriptorPoolResetFlags			ResetFlags = 0;
	};

	/**
	 * @param maxSets 
	 * @param bindings
	 */
	explicit DescriptorAllocator(Device::Ptr pDevice, Options const &option, uint32 maxSets, BindingArray const& bindings);
	~DescriptorAllocator() override;

protected:
	void Initialize(uint32 maxSets, BindingArray const& bindings);
	void Destroy();

private:
	friend class 		DescriptorSet;

	Options				m_Options;
	VkDescriptorPool 	m_Pool;
};

class DescriptorSetLayout : public DeviceChild
{
public:
	DescriptorSetLayout(Device::Ptr pDevice, BindingArray const & bindings);
	~DescriptorSetLayout();

	VkDescriptorSetLayout GetNativeHandle() const { return m_DescriptorSetLayout; }

protected:
	void Initialize(BindingArray const & bindings);
	void Destroy();
private:
	friend class			PipelineLayout;
	VkDescriptorSetLayout	m_DescriptorSetLayout;
	std::vector<VkWriteDescriptorSet> m_UpdateDescSet;
};

class DescriptorSet : public DeviceChild, public rhi::IDescriptor
{
public:
	static DescriptorSet*	CreateDescSet(DescriptorAllocRef descriptorPool, VkDescriptorSetLayout layout, BindingArray const & bindings, Device::Ptr pDevice);
	virtual					~DescriptorSet();
	void					Update(uint32 bindSet, rhi::GpuResourceRef) override;
	VkDescriptorSet			GetNativeHandle() const { return m_DescriptorSet; }

private:
	DescriptorSet( DescriptorAllocRef descriptorPool, VkDescriptorSetLayout layout, BindingArray const & bindings, Device::Ptr pDevice );

	VkDescriptorSet							m_DescriptorSet = VK_NULL_HANDLE;
	DescriptorAllocRef						m_DescriptorAllocator = nullptr;
	BindingArray							m_Bindings;
	std::vector<VkWriteDescriptorSet>		m_BoundDescriptorSet;

	void Initialize( VkDescriptorSetLayout layout, BindingArray const & bindings);
	void Destroy();
};

class Resource : virtual public rhi::IGpuResource, public DeviceChild
{
public:
	typedef void *				Ptr;
	typedef void const*			CPtr;

	explicit					Resource(Device::Ptr pDevice) : DeviceChild(pDevice), m_HostMem(nullptr), m_DeviceMem{}, m_Desc{} {}
								Resource(Device::Ptr pDevice, rhi::ResourceDesc const & desc) : DeviceChild(pDevice), m_HostMem(nullptr), m_DeviceMem{}, m_Desc(desc) {}
	virtual						~Resource();

	Resource::CPtr				GetHostMemory(uint64 OffSet) const { return m_HostMem; }
	VkDeviceMemory				GetDeviceMemory() const { return m_DeviceMem; }

	Resource::Ptr				Map(uint64 offset, uint64 size) override;
	void						UnMap() override { GetGpuRef()->vkUnmapMemory(GetRawDevice(), m_DeviceMem); }
	uint64						GetResourceSize() const override { return m_Size; }
	rhi::ResourceDesc			GetResourceDesc() const override { return m_Desc; }

protected:
	VkMemoryAllocateInfo		m_MemAllocInfo;
	VkDeviceMemory				m_DeviceMem;
	Resource::Ptr				m_HostMem;

	VkDeviceSize				m_Size = 0;
	VkDeviceSize				m_AllocationSize = 0;
	VkDeviceSize				m_AllocationOffset = 0;

	rhi::ResourceDesc			m_Desc;
};

class Buffer : public Resource
{
public:
	typedef Buffer * Ptr;
	explicit					Buffer(Device::Ptr pDevice) : Resource(pDevice) {}
								Buffer(Device::Ptr pDevice, rhi::ResourceDesc const & desc);
	virtual						~Buffer();

	void						Create(size_t size);

	uint64						GetResourceLocation() const override	{ return (uint64)m_Buffer; }
	rhi::EGpuResourceType		GetResourceType() const override		{ return rhi::EGT_Buffer; }
private:
	VkDescriptorBufferInfo		m_BufferInfo;
	VkBufferView				m_BufferView = VK_NULL_HANDLE;
	VkBuffer					m_Buffer = VK_NULL_HANDLE;
	VkBufferUsageFlags  		m_Usage = 0;
	VkMemoryPropertyFlags		m_MemoryBits = 0;
};

class Texture : public rhi::ITexture, public Resource
{
public:
	typedef ::k3d::SharedPtr<Texture> TextureRef;

	explicit					Texture(Device::Ptr pDevice) : Resource(pDevice) {}
								Texture(Device::Ptr pDevice, rhi::ResourceDesc const&);
								Texture(VkImage image, VkImageView imageView, VkImageViewCreateInfo info, Device::Ptr pDevice, bool selfOwnShip = true);
								~Texture() override;
								
	static TextureRef			CreateFromSwapChain(VkImage image, VkImageView view, VkImageViewCreateInfo info, Device::Ptr pDevice);
	
	const VkImageViewCreateInfo&GetViewInfo() const { return m_ImageViewInfo; }
	const VkImageView &			GetView() const { return m_ImageView; }
	const VkImage&				Get() const { return m_Image; }
	VkImageLayout				GetImageLayout() const { return m_ImageLayout; }
	VkImageSubresourceRange		GetSubResourceRange() const { return m_SubResRange; }

	uint64						GetResourceLocation() const override { return (uint64)m_Image; }
	rhi::EResourceState			GetUsageState() const override { return m_UsageState; }

	void						BindSampler(rhi::SamplerRef sampler) override;
	rhi::SamplerCRef			GetSampler() const override;
	rhi::ShaderResourceViewRef	GetResourceView() const override				{ return m_SRV; }
	void						SetResourceView(rhi::ShaderResourceViewRef srv) override { m_SRV = srv; }

	void						CreateResourceView();
	friend class				SwapChain;
	friend class				CommandContext;

private:
	/**
	 * Create texture for Render
	 */
	void						CreateRenderTexture(rhi::TextureDesc const &desc);
	void						CreateDepthStencilTexture(rhi::TextureDesc const &desc);
	/**
	 * Create texture for Sampler
	 */
	void						CreateSampledTexture(rhi::TextureDesc const &desc);

private:
	::k3d::SharedPtr<Sampler>	m_ImageSampler;
	VkImageViewCreateInfo		m_ImageViewInfo = {};
	rhi::ShaderResourceViewRef	m_SRV;
	VkImageView					m_ImageView = VK_NULL_HANDLE;
	ImageInfo  					m_ImageInfo;
	VkImage						m_Image = VK_NULL_HANDLE;
	rhi::EResourceState			m_UsageState = rhi::ERS_Unknown;
	VkImageLayout				m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageMemoryBarrier		m_Barrier;
	VkImageUsageFlags			m_ImageUsage = 0;
	VkMemoryPropertyFlags		m_MemoryBits = 0;
	VkSubresourceLayout			m_SubResourceLayout = {};
	VkImageSubresourceRange     m_SubResRange = {};
	bool						m_SelfOwn = true;
};


class ResourceManager : public DeviceChild
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

		static PoolPtr			Create(VkDevice device, const VkDeviceSize poolSize, const typename ResourceManager::ResDesc<VkObject> & objDesc);

		uint32					GetMemoryTypeIndex() const { return m_MemoryTypeIndex; }
		VkDeviceSize			GetSize() const { return m_Size; }
		bool					HasAvailable(VkMemoryRequirements memReqs) const;


		Allocation				Allocate(const typename ResourceManager::ResDesc<VkObject> & objDesc);
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
		Allocation						Allocate(const typename ResourceManager::ResDesc<VkObjectT>& objDesc);

		VkDeviceSize					GetBlockSize() const { return m_PoolSize; }
	private:
		VkDevice						m_Device = VK_NULL_HANDLE;
		size_t							m_PoolSize = 0;
		::Os::Mutex						m_Mutex;
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

	rhi::CommandContextRef RequestContext(rhi::ECommandType type);

	PtrCmdAlloc	RequestCommandAllocator();

private:
	using Mutex = Os::Mutex;
	Mutex m_PoolMutex;
	Mutex m_ContextMutex;
	std::unordered_map<uint32, PtrCmdAlloc> m_AllocatorPool;
	std::unordered_map<uint32, std::list<CommandContext*> > m_ContextList;
};
/**
 * TODO: Need a Renderpass manager to cache all renderpasses
 */
class RenderTarget : public DeviceChild, public rhi::IRenderTarget
{
public:
	RenderTarget(Device::Ptr pDevice, rhi::RenderTargetLayout const& Layout);
	RenderTarget(Device::Ptr pDevice, Texture::TextureRef texture, SpFramebuffer framebuffer, VkRenderPass renderpass);
	~RenderTarget() override;

	VkFramebuffer		GetFramebuffer() const;
	VkRenderPass		GetRenderpass() const;
	Texture::TextureRef	GetTexture() const;
	VkRect2D			GetRenderArea() const;
	rhi::GpuResourceRef	GetBackBuffer() override;
	PtrSemaphore		GetSemaphore() { return m_AcquireSemaphore; }

	void				SetClearColor(kMath::Vec4f clrColor) override { m_ClearValues[0].color = { clrColor[0], clrColor[1], clrColor[2], clrColor[3] }; }
	void				SetClearDepthStencil(float depth, uint32 stencil) override { m_ClearValues[1].depthStencil = {depth, stencil}; }

private:
	friend class	CommandContext;

	VkClearValue	m_ClearValues[2] = { {}, {1.0f, 0} };

	SpFramebuffer	m_Framebuffer;
	VkRenderPass	m_Renderpass;
	Texture::TextureRef	m_RenderTexture;
	PtrSemaphore	m_AcquireSemaphore;
};

/**
 * Need CommandBufferManager to issue Cmds
 */
class CommandContext : public rhi::ICommandContext, public DeviceChild
{
public:
	explicit			CommandContext(Device::Ptr pDevice);
						CommandContext(Device::Ptr pDevice, VkCommandBuffer cmdBuf, VkCommandPool pool, rhi::ECommandType type = rhi::ECMD_Graphics);
	virtual				~CommandContext();

	void				Detach(rhi::IDevice *) override;
	void				CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src, rhi::CopyBufferRegion const& Region) override;
	void				CopyTexture(const rhi::TextureCopyLocation& Dest, const rhi::TextureCopyLocation& Src);
	/**
	 * @brief	submit command buffer to queue and wait for scheduling.
	 * @param	Wait	true to wait.
	 */
	void				Execute(bool Wait) override;
	void				Reset() override;

	void				SubmitAndWait(PtrSemaphore wait, PtrSemaphore signal, PtrFence fence);

	void				Begin() override;
	void				End() override;

	void				BeginRendering() override;
	void				EndRendering() override;

	void				PresentInViewport(rhi::RenderViewportRef) override;

	void				BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
		uint32 firstSet, uint32 descriptorSetCount, const VkDescriptorSet* pDescriptorSets,
		uint32 dynamicOffsetCount, const uint32* pDynamicOffsets);
	void				BindDescriptorSet(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, const VkDescriptorSet& pDescriptorSets);
	void				ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor,
		uint32 rangeCount, const VkImageSubresourceRange* pRanges);

	void				ClearColorImage(SpTexture colorBuffer, const VkClearColorValue* pColor, VkImageLayout imageLayout);

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

	void				SetRenderTarget(rhi::RenderTargetRef rt) override;
	void				ClearColorBuffer(rhi::GpuResourceRef, kMath::Vec4f const&)override;
	void				ClearDepthBuffer(rhi::IDepthBuffer* iDepthBuffer)override;
	void				SetRenderTargets(uint32 NumColorBuffer, rhi::IColorBuffer *, rhi::IDepthBuffer *, bool ReadOnlyDepth = false) override;
	void				SetViewport(const rhi::ViewportDesc &)override;
	void				SetScissorRects(uint32, const rhi::Rect*)override;
	void				SetIndexBuffer(const rhi::IndexBufferView& IBView) override;
	void				SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView) override;
	void				SetPipelineState(uint32 hashCode, rhi::PipelineStateObjectRef)override;
	void				SetPipelineLayout(rhi::PipelineLayoutRef pRHIPipelineLayout) override;
	void				SetPrimitiveType(rhi::EPrimitiveType)override;
	void				DrawInstanced(rhi::DrawInstancedParam)override;
	void				DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)override;
	void				Dispatch(uint32 X, uint32 Y, uint32 Z) override;
	void				TransitionResourceBarrier(rhi::GpuResourceRef resource,/* rhi::EPipelineStage stage,*/ rhi::EResourceState dstState) override;
	friend class			CommandContextPool;

	void				ExecuteBundle(rhi::ICommandContext * ) override;

protected:
	VkCommandBuffer			m_CommandBuffer;
	//VkCommandPool			m_CommandPool;
	VkRenderPass			m_RenderPass;
	bool					m_IsRenderPassActive = false;
	RenderTarget*			m_CurrentRenderTarget = nullptr;
	rhi::ECommandType		m_CmdType = rhi::ECMD_Graphics;
private:
	void					InitCommandBufferPool();
};

class CommandQueue : public DeviceChild
{
public:
			CommandQueue(Device::Ptr pDevice, VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex);
	virtual ~CommandQueue();

	void Submit(
		const std::vector<VkCommandBuffer>& cmdBufs, 
		const std::vector<VkSemaphore>& waitSemaphores, 
		const std::vector<VkPipelineStageFlags>& waitStageMasks, 
		VkFence fence, 
		const std::vector<VkSemaphore>& signalSemaphores);

	VkResult Submit(const std::vector<VkSubmitInfo>& submits, VkFence fence);
	
	void WaitIdle();

	VkQueue GetNativeHandle() const { return m_Queue; }

protected:
	void Initialize(VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex);
	void Destroy();

private:
	VkQueue			m_Queue = VK_NULL_HANDLE;
	VkQueueFlags	m_QueueTypes = 0;
	uint32			m_QueueFamilyIndex = UINT32_MAX;
	uint32			m_QueueIndex = UINT32_MAX;
};

class SwapChain : public DeviceChild
{
public:
	DEVICE_CHILD_CONSTRUCT_DECLARE(SwapChain)
		~SwapChain();

	using									TexRefList = std::vector<SpTexture>;

	void									Initialize(
												void* WindowHandle, 
												rhi::GfxSetting & gfxSetting);

	uint32									GetPresentQueueFamilyIndex() const { return m_SelectedPresentQueueFamilyIndex; }
	uint32									AcquireNextImage(PtrSemaphore presentSemaphore, PtrFence pFence);
	VkResult								Present(uint32 imageIndex, PtrSemaphore renderingFinishSemaphore);
	uint32									GetBackBufferCount() const { return m_ReserveBackBufferCount; }
	
	VkSwapchainKHR							GetSwapChain() const { return m_SwapChain; }
	VkImage									GetBackImage(uint32 i) const { return m_ColorImages[i]; }

	VkExtent2D								GetCurrentExtent() const { return m_SwapchainExtent; }
	VkFormat								GetFormat() const { return m_ColorAttachFmt; }

private:
	void									InitSurface(void * WindowHandle);
	VkPresentModeKHR						ChoosePresentMode();
	std::pair<VkFormat, VkColorSpaceKHR>	ChooseFormat(rhi::GfxSetting & gfxSetting);
	int										ChooseQueueIndex();
	void									InitSwapChain(uint32 numBuffers, std::pair<VkFormat, VkColorSpaceKHR> color, VkPresentModeKHR mode, VkSurfaceTransformFlagBitsKHR pretran);

private:
	VkExtent2D								m_SwapchainExtent = {};
	VkSurfaceKHR							m_Surface	= VK_NULL_HANDLE;
	VkSwapchainKHR							m_SwapChain = VK_NULL_HANDLE;
	uint32									m_SelectedPresentQueueFamilyIndex = 0;
	uint32 									m_DesiredBackBufferCount;
	uint32									m_ReserveBackBufferCount;
	std::vector<VkImage>					m_ColorImages;
	VkFormat								m_ColorAttachFmt = VK_FORMAT_UNDEFINED;

private:
	void Destroy();
};

class RenderViewport : public rhi::IRenderViewport, public DeviceChild
{
public:
						RenderViewport(Device::Ptr, void* windowHandle, rhi::GfxSetting &);
						~RenderViewport() override; 
	bool				InitViewport(void *windowHandle, rhi::IDevice * pDevice,
								rhi::GfxSetting &) override;

	void				PrepareNextFrame() override;

	bool				Present(bool vSync) override;

	rhi::RenderTargetRef GetRenderTarget(uint32 index) override;
	rhi::RenderTargetRef GetCurrentBackRenderTarget() override;

	uint32				GetSwapChainIndex() override;
	uint32				GetSwapChainCount() override;

	PtrSemaphore		GetPresentSemaphore() const { return m_PresentSemaphore; }
	PtrSemaphore		GetRenderSemaphore() const { return m_RenderSemaphore; }

	void				AllocateDefaultRenderPass(rhi::GfxSetting & gfxSetting);
	void				AllocateRenderTargets(rhi::GfxSetting & gfxSetting);
	VkRenderPass		GetRenderPass() const;

	uint32				GetWidth() const override { return m_pSwapChain->GetCurrentExtent().width; }
	uint32				GetHeight() const override { return m_pSwapChain->GetCurrentExtent().height; }
protected:

	PtrSemaphore					m_PresentSemaphore;
	PtrSemaphore					m_RenderSemaphore;
	SpRenderpass					m_RenderPass;

private:
	std::vector<rhi::RenderTargetRef>m_RenderTargets;

	uint32							m_CurFrameId;
	SwapChainRef					m_pSwapChain;
	uint32							m_NumBufferCount;
};

class RenderPass;
class FrameBuffer : public DeviceChild
{
public:

	struct Attachment
	{
		Attachment(VkFormat format, VkSampleCountFlagBits samples);
		explicit Attachment(VkImageView view) { this->ImageAttachment = view; }

		virtual					~Attachment() {}


		bool					IsColorAttachment() const { return 0 != (FormatFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT); }
		bool					IsDepthStencilAttachment() const { return 0 != (FormatFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); }

		VkFormat				Format = VK_FORMAT_UNDEFINED;
		VkFormatFeatureFlags	FormatFeatures = 0;
		VkSampleCountFlagBits	Samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageView				ImageAttachment;
	};


	struct Option
	{
		uint32						Width, Height;
		std::vector<Attachment>		Attachments;
	};

	/**
	* create framebuffer with SwapChain ImageViews
	*/
	FrameBuffer(Device::Ptr pDevice, VkRenderPass renderPass, Option const& op);
	FrameBuffer(Device::Ptr pDevice, RenderPass* renderPass, RenderTargetLayout const &);
	~FrameBuffer();

	uint32					GetWidth() const { return m_Width; }
	uint32					GetHeight() const { return m_Height; }
	VkFramebuffer const		Get() const { return m_FrameBuffer; }

private:
	friend class			RenderPass;

	VkFramebuffer			m_FrameBuffer = VK_NULL_HANDLE;
	VkRenderPass			m_RenderPass = VK_NULL_HANDLE;
	uint32					m_Width = 0;
	uint32					m_Height = 0;
};

using PtrFrameBuffer = std::shared_ptr<FrameBuffer>;

class RenderPass : public DeviceChild
{
public:
	typedef RenderPass	*		Ptr;

	RenderPass(Device::Ptr pDevice, RenderpassOptions const & options);
	RenderPass(Device::Ptr pDevice, RenderTargetLayout const & rtl);
	~RenderPass();

	void								NextSubpass();

	VkRenderPass						GetPass() const { return m_RenderPass; }
	RenderpassOptions const &			GetOption() const { return m_Options; }

private:
	RenderpassOptions					m_Options;
	PtrContext							m_GfxContext;
	PtrFrameBuffer						m_FrameBuffer;
	VkRenderPass						m_RenderPass = VK_NULL_HANDLE;

	uint32								mSubpass = 0;
	std::vector<VkSampleCountFlagBits>	mSubpassSampleCounts;
	std::vector<uint32>					mBarrieredAttachmentIndices;
	std::vector<VkAttachmentDescription>mAttachmentDescriptors;
	std::vector<VkClearValue>			mAttachmentClearValues;

	void				Initialize(RenderpassOptions const & options);
	void				Initialize(RenderTargetLayout const & rtl);
	void				Destroy();

	friend class		FrameBuffer;
};


class PipelineStateObject : public rhi::IPipelineStateObject, public DeviceChild
{
public:
	explicit 							PipelineStateObject(Device* pDevice);
										PipelineStateObject(Device::Ptr pDevice, rhi::PipelineDesc const& desc, PipelineLayout * ppl);
	virtual								~PipelineStateObject();

	void								BindRenderPass(VkRenderPass RenderPass);
	void								SetShader(rhi::EShaderType, rhi::ShaderBundle const&) override;
	void								SetLayout(rhi::PipelineLayoutRef) override;

	void								SetRasterizerState(const rhi::RasterizerState&)override;
	void								SetBlendState(const rhi::BlendState&)override;
	void								SetDepthStencilState(const rhi::DepthStencilState&)override;
	void								SetSampler(rhi::SamplerRef)override;
	void								SetVertexInputLayout(rhi::VertexDeclaration const*, uint32 Count) override;
	void								SetPrimitiveTopology(const rhi::EPrimitiveType) override;
	void								SetRenderTargetFormat(const rhi::RenderTargetFormat &) override;

	VkPipeline 							GetPipeline() const { return m_Pipeline; }
	void								Finalize() override;
	void								SavePSO(const char* path) override;
	void								LoadPSO(const char* path) override;

	/**
	 * TOFIX
	 */
	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Graphics;
	}

protected:
	void											InitWithDesc(rhi::PipelineDesc const & desc);
	void											Destroy();

	friend class									CommandContext;

	VkPipeline										m_Pipeline;
	VkPipelineCache									m_PipelineCache;
	union 
	{
		VkGraphicsPipelineCreateInfo				m_GfxCreateInfo;
		VkComputePipelineCreateInfo					m_CptCreateInfo;
	};
	VkRenderPass									m_RenderPass;

private:
	std::vector<VkPipelineShaderStageCreateInfo>	m_ShaderStageInfos;
	VkPipelineInputAssemblyStateCreateInfo			m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo			m_RasterizationState;
	VkPipelineColorBlendStateCreateInfo				m_ColorBlendState;
	VkPipelineDepthStencilStateCreateInfo			m_DepthStencilState;
	VkPipelineViewportStateCreateInfo				m_ViewportState;
	VkPipelineMultisampleStateCreateInfo			m_MultisampleState;
	VkPipelineVertexInputStateCreateInfo			m_VertexInputState;
	std::vector<VkVertexInputBindingDescription>	m_BindingDescriptions;
	std::vector<VkVertexInputAttributeDescription>	m_AttributeDescriptions;
	PipelineLayout *								m_PipelineLayout;
};

class ShaderResourceView : public rhi::IShaderResourceView
{
public:
	ShaderResourceView(rhi::ResourceViewDesc const &desc, rhi::GpuResourceRef gpuResource);
	~ShaderResourceView();
	rhi::GpuResourceRef		GetResource() const override { return m_Resource; }
	rhi::ResourceViewDesc	GetDesc() const override { return m_Desc; }
	VkImageView				NativeImageView() const { return m_TextureView; }
private:
	rhi::GpuResourceRef		m_Resource;
	rhi::ResourceViewDesc	m_Desc;
	VkImageViewCreateInfo	m_TextureViewInfo;
	VkImageView				m_TextureView;
};

class PipelineLayout : public rhi::IPipelineLayout, public DeviceChild
{
public:
	PipelineLayout(Device::Ptr pDevice, rhi::PipelineLayoutDesc const &desc);
	~PipelineLayout()override;

	VkPipelineLayout	GetNativeLayout() const			{ return m_PipelineLayout; }
	VkDescriptorSet		GetNativeDescriptorSet() const	{ return static_cast<DescriptorSet*>(m_DescSet.Get())->GetNativeHandle(); }

	rhi::DescriptorRef	GetDescriptorSet()const override{ return m_DescSet; }

protected:
	void				InitWithDesc(rhi::PipelineLayoutDesc const &desc);
	void				Destroy();
	friend class		PipelineStateObject;
private:
	rhi::DescriptorRef		m_DescSet;
	DescriptorSetLayoutRef	m_DescSetLayout;
	VkPipelineLayout		m_PipelineLayout;
};

class Sampler : public rhi::ISampler, public DeviceChild
{
public:
	explicit			Sampler(Device::Ptr pDevice, rhi::SamplerState const & sampleDesc);
						~Sampler() override;
	rhi::SamplerState	GetSamplerDesc() const;
	VkSampler			NativePtr() const { return m_Sampler; }
protected:
	VkSampler 			m_Sampler = VK_NULL_HANDLE;
	VkSamplerCreateInfo m_SamplerCreateInfo = {};
	rhi::SamplerState   m_SamplerState;
};

K3D_VK_END

#endif