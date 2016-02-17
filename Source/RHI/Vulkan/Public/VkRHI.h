#ifndef __VkRHI_h__
#define __VkRHI_h__
#pragma once

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
								Device() {}
								~Device() {}
	Result						Create(rhi::IDeviceAdapter *, bool withDebug) override;

	rhi::ICommandContext*		NewCommandContext(rhi::ECommandType)override;
	rhi::IGpuResource*			NewGpuResource(rhi::EGpuResourceType type)override;
	rhi::ISampler*				NewSampler(const rhi::SamplerState&)override;
	rhi::IPipelineStateObject*	NewPipelineState(rhi::EPipelineType)override;
	rhi::ISyncPointFence*		NewFence()override;

private:
	VkDevice m_Device;
};

class CommandContext : public rhi::ICommandContext
{
public:
	CommandContext() {}
			~CommandContext() {}

	void	Detach(rhi::IDevice *) = 0;
	void	CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src) = 0;
	void	Execute(bool Wait) = 0;
	void	Reset() = 0;
};

class PipelineStateObject : public rhi::IPipelineStateObject
{
public:

	virtual ~PipelineStateObject();

	void	SetShader(rhi::EShaderType, rhi::IShaderBytes*)override;
	void	SetLayout(rhi::IPipelineLayout *) override;
	void	Finalize() override;
	virtual rhi::EPipelineType	GetType() = 0;
};

class GraphicsPSO : public rhi::IGraphicsPipelineState, public PipelineStateObject
{
public:
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
};

class ComputePSO : public rhi::IComputePipelineState, public PipelineStateObject
{
public:

	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Compute;
	}
};

K3D_VK_END

#endif