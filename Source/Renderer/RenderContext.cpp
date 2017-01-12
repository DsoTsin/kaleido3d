#include "Kaleido3D.h"
#include <Core/Module.h>
#if K3DPLATFORM_OS_WIN
#include <RHI/Vulkan/VkCommon.h>
#include <RHI/Vulkan/Public/IVkRHI.h>
#elif K3DPLATFORM_OS_MAC||K3DPLATFORM_OS_IOS
#include <RHI/Metal/Common.h>
#include <RHI/Metal/Public/IMetalRHI.h>
#else
#include <RHI/Vulkan/VkCommon.h>
#include <RHI/Vulkan/Public/IVkRHI.h>
#endif
#include "Core/LogUtil.h"
#include "Render.h"

using namespace k3d;

namespace render
{
    const char * TAG_RENDERCONTEXT = "RenderContext";
    
    RenderContext::RenderContext()
		: m_RenderVp(nullptr)
    {
    }
    
    void RenderContext::Init(RHIType type, uint32 w, uint32 h)
    {
		m_Width = w;
		m_Height = h;
		m_RhiType = type;
#if !(K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_IOS)
		IVkRHI* pVkRHI = (IVkRHI*)ACQUIRE_PLUGIN(RHI_Vulkan);
		pVkRHI->Initialize("RenderContext", true);
		pVkRHI->Start();
		m_pDevice = pVkRHI->GetPrimaryDevice();
#else 
        IMetalRHI* pMtlRHI = (IMetalRHI*)ACQUIRE_PLUGIN(RHI_Metal);
        if(pMtlRHI)
        {
            pMtlRHI->Start();
            m_pDevice = pMtlRHI->GetPrimaryDevice();
        }
#endif
    }

	void RenderContext::Attach(::k3d::IWindow::Ptr hostWindow)
	{
		if (!m_RenderVp)
		{
			rhi::GfxSetting setting{m_Width, m_Height, rhi::EPF_RGBA8Unorm, rhi::EPF_D32Float, true, 2 };
			m_RenderVp = m_pDevice->NewRenderViewport(hostWindow->GetHandle(), setting);
			m_RenderVp->InitViewport(nullptr, nullptr, setting);
		}
	}

	void RenderContext::PrepareRenderingResource()
	{
	}

	void RenderContext::PreRender()
	{
	}

	void RenderContext::Render()
	{
		if (m_RenderVp)
		{
			m_RenderVp->PrepareNextFrame();
			m_RenderVp->Present(false);
		}
	}

	void RenderContext::PostRender()
	{
	}
    
    void RenderContext::Destroy()
    {
		KLOG(Info, "RenderContext", "Destroyed Render Context.");
    }
    
    RenderContext::~RenderContext()
    {
    }
}
