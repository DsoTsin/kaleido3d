#ifndef __RenderContext_h__
#define __RenderContext_h__
#pragma once
#include "RHI/IRHI.h"
#include <memory>

namespace k3d {
    
    using PtrRHIDevice = std::shared_ptr<rhi::IDevice>;
    
    /**
     Associated with RHI
     */
    class RenderContext
    {
    public:
        RenderContext();
        
        void Init();
        void Destroy();
        
        ~RenderContext();
        
    private:
        
        PtrRHIDevice m_pDevice;
    };
    
}


#endif