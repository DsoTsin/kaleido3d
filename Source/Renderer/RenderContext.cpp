#include "Kaleido3D.h"
#include "Core/LogUtil.h"
#include "RenderContext.h"

namespace k3d
{
    const char * TAG_RENDERCONTEXT = "RenderContext";
    
    RenderContext::RenderContext()
    {
        
    }
    
    void RenderContext::Init()
    {
        rhi::IDeviceAdapter ** adapters = nullptr;
        uint32 adapterCount = 0;
        rhi::EnumAllDeviceAdapter(adapters, &adapterCount);
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