#include "Kaleido3D.h"
#if K3DPLATFORM_OS_WIN
#include <RHI/D3D12/DXCommon.h>
#include <RHI/D3D12/Public/D3D12RHI.h>
#include <RHI/Vulkan/VkCommon.h>
#include <RHI/Vulkan/Public/VkRHI.h>
#endif
#include "Core/LogUtil.h"
#include "RenderContext.h"


namespace k3d
{
    const char * TAG_RENDERCONTEXT = "RenderContext";
    
	typedef void(*PFNRHIEnumFunc)(rhi::IDeviceAdapter**&,uint32*);
	typedef rhi::IRenderViewport * (*PFNAllocateRHIRenderViewport)(rhi::IDevice* pDevice, void* WindowHandle);

	PFNRHIEnumFunc s_RHIEnumFunction[] = {
		nullptr,
		reinterpret_cast<PFNRHIEnumFunc>(vk::EnumAllDeviceAdapter),
		reinterpret_cast<PFNRHIEnumFunc>(d3d12::EnumAllDeviceAdapter)
	};

	PFNAllocateRHIRenderViewport s_RHIAllocateViewportFn[] = {
		nullptr,
		reinterpret_cast<PFNAllocateRHIRenderViewport>(vk::AllocateRHIRenderViewport),
		reinterpret_cast<PFNAllocateRHIRenderViewport>(d3d12::AllocateRHIRenderViewport)
	};

    RenderContext::RenderContext()
    {
        
    }
    
    void RenderContext::Init(RHIType type)
    {
		m_RhiType = type;
        rhi::IDeviceAdapter ** adapters = nullptr;
        uint32 adapterCount = 0;
		s_RHIEnumFunction[static_cast<uint32>(type)](adapters, &adapterCount);
		m_pDevice = adapters[0]->GetDevice();
        rhi::IDevice::Result result = m_pDevice->Create(adapters[0], false);
        Log::Out(TAG_RENDERCONTEXT, result == rhi::IDevice::DeviceFound ?
                 "Device found !" : "Device unfound...");
		//s_RHIAllocateViewportFn[static_cast<uint32>(type)](m_pDevice.get(), nullptr);
    }
    
    void RenderContext::Destroy()
    {
    }
    
    RenderContext::~RenderContext()
    {
        
    }

	rhi::IRenderViewport * RenderContext::AllocateRenderVP()
	{
		return nullptr;
	}
    
}