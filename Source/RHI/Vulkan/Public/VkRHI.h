#ifndef __VkRHI_h__
#define __VkRHI_h__
#pragma once

class VulkanSwapChain;

K3D_VK_BEGIN

extern void EnumAllDeviceAdapter(rhi::IDeviceAdapter** &, uint32*);

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
	static Initializer					s_Impl;
};


class DeviceAdapter : public rhi::IDeviceAdapter
{
	friend class Device;
public:
	explicit DeviceAdapter(VkPhysicalDevice * pDevice) : m_pPDevice(pDevice) {}

	rhi::IDevice * GetDevice() override
	{
		return nullptr;
	}
private:
	VkPhysicalDevice * m_pPDevice;
};

class Device : public rhi::IDevice 
{
public:
	typedef Device * Ptr;

								Device() : m_pPhysicDevice(nullptr) {}
								~Device();
	Result						Create(rhi::IDeviceAdapter *, bool withDebug) override;

	rhi::ICommandContext*		NewCommandContext(rhi::ECommandType)override;
	rhi::IGpuResource*			NewGpuResource(rhi::EGpuResourceType type)override;
	rhi::ISampler*				NewSampler(const rhi::SamplerState&)override;
	rhi::IPipelineStateObject*	NewPipelineState(rhi::EPipelineType)override;
	rhi::ISyncPointFence*		NewFence()override;


	VkQueue const&				GetRawDeviceQueue() const { return m_DefaultQueue; }
	VkDevice const&				GetRawDevice() const { return m_Device; }
	VkPhysicalDevice const*		GetRawPhysicDevice() const { return m_pPhysicDevice; }

private:
	VkQueue						m_DefaultQueue;
	VkDevice					m_Device;
	VkPhysicalDevice *			m_pPhysicDevice;
};

class DeviceChild
{
public:
	explicit					DeviceChild(Device *pDevice) : m_pDevice(pDevice) {}
	VkDevice const &			GetRawDevice() const { return m_pDevice->GetRawDevice(); }
private:
	Device::Ptr					m_pDevice;
};

class Resource : public rhi::IGpuResource, public DeviceChild
{
public:
	typedef void *				Ptr;
	typedef void const*			CPtr;

	explicit					Resource(Device::Ptr pDevice) : DeviceChild(pDevice), m_HostMem(nullptr), m_DeviceMem{} {}
	virtual						~Resource();

	Resource::CPtr				GetHostMemory(uint64 OffSet) const { return m_HostMem; }
	VkDeviceMemory				GetDeviceMemory() const { return m_DeviceMem; }

protected:
	VkDeviceMemory				m_DeviceMem;
	Resource::Ptr				m_HostMem;
};

class Buffer : public Resource
{
public:
	explicit					Buffer(Device::Ptr pDevice) : Resource(pDevice) {}
								~Buffer() override;

private:
	VkBufferView				m_BufferView;
	VkBuffer					m_Buffer;
};

class Texture : public Resource
{
public:
	explicit					Texture(Device::Ptr pDevice) : Resource(pDevice) {}
								~Texture() override;
private:
	VkImageView					m_ImageView;
	VkImage						m_Image;
};

class CommandContext : public rhi::ICommandContext, public DeviceChild
{
public:
	using CmdBufLst =	std::vector<VkCommandBuffer>;
	explicit			CommandContext(Device::Ptr pDevice) : DeviceChild(pDevice) { InitCommandBufferPool(); }
	virtual				~CommandContext();

	void				Detach(rhi::IDevice *) override;
	void				CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src) override;
	void				Execute(bool Wait) override;
	void				Reset() override;

protected:
	VkCommandPool		m_CommandPool;
	//CmdBufLst			m_CommandList;

private:
	void				InitCommandBufferPool();
};

class RenderViewport : public rhi::IRenderViewport
{
public:
						RenderViewport();
						~RenderViewport() override; 
	bool				InitViewport(
							void *windowHandle,
							rhi::IDevice * pDevice,
							uint32 width, uint32 height,
							rhi::EPixelFormat rtFmt
							) override;
	bool				Present(bool vSync) override;
private:
	std::unique_ptr<VulkanSwapChain>	m_pSwapChain;
};

class PipelineLayout : public rhi::IPipelineLayout
{
public:
	void Create(rhi::ShaderParamLayout const &) override;
	void Finalize(rhi::IDevice *) override;

private:
	VkPipelineLayout m_PipelineLayout;
};

struct Compiler : public rhi::IShaderCompiler, public DeviceChild
{
	explicit				Compiler(Device::Ptr pDevice) : DeviceChild(pDevice) {}
	rhi::IShaderBytes*		CompileFromSource(ELangVersion, rhi::EShaderType, const char*) override;
	rhi::IShaderBytes*		CompileFromSource(ELangVersion, rhi::EShaderType, const char*, const char * entry);
};

class PipelineStateObject : public rhi::IPipelineStateObject, public DeviceChild
{
public:

	explicit							PipelineStateObject(Device *pDevice);
	virtual								~PipelineStateObject();

	void								BindRenderPass(VkRenderPass RenderPass);
	void								SetShader(rhi::EShaderType, rhi::IShaderBytes*) override;
	void								SetLayout(rhi::IPipelineLayout *) override;
	void								Finalize() override;
	virtual rhi::EPipelineType			GetType() = 0;

protected:
	
	VkPipeline							m_Pipeline;
	VkPipelineCache						m_PipelineCache;
	union 
	{
		VkGraphicsPipelineCreateInfo	m_GfxCreateInfo;
		VkComputePipelineCreateInfo		m_CptCreateInfo;
	};
	VkRenderPass						m_RenderPass;
};

class GraphicsPSO : public rhi::IGraphicsPipelineState, public PipelineStateObject
{
public:
	explicit GraphicsPSO(Device::Ptr pDevice);
			~GraphicsPSO() override;

	void	SetRasterizerState(const rhi::RasterizerState&)override;
	void	SetBlendState(const rhi::BlendState&)override;
	void	SetDepthStencilState(const rhi::DepthStencilState&)override;
	void	SetSampler(rhi::ISampler*)override;
	void	SetVertexInputLayout(rhi::VertexDeclaration *, uint32 Count) override;
	void	SetPrimitiveTopology(const rhi::EPrimitiveType) override;
	void	SetRenderTargetFormat(const rhi::RenderTargetFormat &) override;

	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Graphics;
	}

private:
	VkPipelineInputAssemblyStateCreateInfo	m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo	m_RasterizationState;
	VkPipelineColorBlendStateCreateInfo		m_ColorBlendState;
	VkPipelineDepthStencilStateCreateInfo	m_DepthStencilState;
	VkPipelineViewportStateCreateInfo		m_ViewportState;
	VkPipelineMultisampleStateCreateInfo	m_MultisampleState;
};

class ComputePSO : public rhi::IComputePipelineState, public PipelineStateObject
{
public:
	explicit ComputePSO(Device::Ptr pDevice);
	~ComputePSO() override;

	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Compute;
	}
};

class GraphicsCommandContext : virtual public rhi::IGraphicsCommand, virtual public CommandContext
{
public:
	explicit			GraphicsCommandContext(Device::Ptr pDevice) : CommandContext(pDevice) {}
						~GraphicsCommandContext() override {}

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

protected:
	void				InitCommandBufferAndPass();

private:
	VkCommandBuffer		m_CommandBuffer;
	VkRenderPass		m_RenderPass;
}; 

class ComputeCommandContext : virtual public rhi::IComputeCommand, virtual public CommandContext
{
public:
	explicit ComputeCommandContext(Device::Ptr pDevice) : CommandContext(pDevice) {}
	~ComputeCommandContext() override {}
	void SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout) override;
	void Dispatch(uint32 X, uint32 Y, uint32 Z) override;
};

K3D_VK_END

#endif