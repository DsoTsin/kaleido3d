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
    
    RenderContext::RenderContext()
    {
        
    }
    
    void RenderContext::Init(RHIType type)
    {
        rhi::IDeviceAdapter ** adapters = nullptr;
        uint32 adapterCount = 0;
		switch (type) 
		{
		case RHIType::ERTDirect3D12:
			d3d12::EnumAllDeviceAdapter(adapters, &adapterCount);
			break;
		case RHIType::ERTVulkan:
			vk::EnumAllDeviceAdapter(adapters, &adapterCount);
			break;
		case RHIType::ERTMetal:
			break;
		}
		m_pDevice = std::shared_ptr<rhi::IDevice>(adapters[0]->GetDevice());
        rhi::IDevice::Result result = m_pDevice->Create(adapters[0], false);
        Log::Out(TAG_RENDERCONTEXT, result == rhi::IDevice::DeviceFound ?
                 "Device found !" : "Device unfound...");
    }
    
    void RenderContext::Destroy()
    {
    }
    
    RenderContext::~RenderContext()
    {
        
    }
    
}