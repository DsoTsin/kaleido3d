#include "Renderer.h"
#include <Core/Module.h>
#if K3DPLATFORM_OS_WIN
#include <RHI/Vulkan/Public/IVkRHI.h>
#include <RHI/Vulkan/VkCommon.h>
#elif K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_IOS
#include <RHI/Metal/Common.h>
#include <RHI/Metal/Public/IMetalRHI.h>
#else
#include <RHI/Vulkan/Public/IVkRHI.h>
#include <RHI/Vulkan/VkCommon.h>
#endif
#include "Core/LogUtil.h"

using namespace k3d;

namespace render {
const char* TAG_RENDERCONTEXT = "RenderContext";

RenderContext::RenderContext()
{
}

void
RenderContext::Init(RHIType type, uint32 w, uint32 h)
{
  m_Width = w;
  m_Height = h;
  m_RhiType = type;
#if !(K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_IOS)
  auto pVkRHI = k3d::StaticPointerCast<IVkRHI>(ACQUIRE_PLUGIN(RHI_Vulkan));
  pVkRHI->Initialize("RenderContext", true);
  pVkRHI->Start();
  auto pFactory = pVkRHI->GetFactory();
  DynArray<NGFXDeviceRef> Devices;
  pFactory->EnumDevices(Devices);
  m_pDevice = Devices[0];
#else
  auto pMtlRHI = k3d::StaticPointerCast<IMetalRHI>(ACQUIRE_PLUGIN(RHI_Metal));
  if (pMtlRHI) {
    pMtlRHI->Start();
    m_pDevice = pMtlRHI->GetPrimaryDevice();
  }
#endif
}

void
RenderContext::Attach(::k3d::IWindow::Ptr hostWindow)
{
}

void
RenderContext::PrepareRenderingResource()
{
}

void
RenderContext::PreRender()
{
}

void
RenderContext::Render()
{
}

void
RenderContext::PostRender()
{
}

void
RenderContext::Destroy()
{
  m_pDevice->WaitIdle();
  KLOG(Info, "RenderContext", "Destroyed Render Context.");
}

RenderContext::~RenderContext()
{
}
}
