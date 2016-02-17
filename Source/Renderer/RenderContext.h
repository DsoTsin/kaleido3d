#ifndef __RenderContext_h__
#define __RenderContext_h__
#pragma once
#include <memory>

namespace rhi 
{
	struct IDevice;
}

namespace k3d 
{
    
	enum class RHIType
	{
		ERTMetal,
		ERTVulkan,
		ERTDirect3D12,
	};

    using PtrRHIDevice = std::shared_ptr<rhi::IDevice>;
    
    /**
     Associated with RHI
     */
    class RenderContext
    {
    public:
        RenderContext();
        
        void Init(RHIType type = RHIType::ERTVulkan);
        void Destroy();
        
        ~RenderContext();
        
    private:
        
        PtrRHIDevice m_pDevice;
    };
    
}


#endif