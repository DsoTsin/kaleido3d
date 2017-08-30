#include <Core/App.h>
#include <Core/Message.h>
#include <Core/Interface/IRHI.h>
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

using namespace k3d;

class SwapchainPresent : public App {
public:
  explicit SwapchainPresent(String const &appName)
      : App(appName, 1920, 1080) {}
  SwapchainPresent(String const &appName, uint32 width, uint32 height)
      : App(appName, width, height) {}

  bool OnInit() override;
  void OnDestroy() override;
  void OnProcess(Message &msg) override;

private:
  SharedPtr<IModule> m_pRHI;
  NGFXFactoryRef m_pFactory;

  void InitRHIObjects();

  NGFXDeviceRef m_pDevice;
  NGFXCommandQueueRef m_pCmdQueue;
  NGFXSwapChainRef m_pSwapChain;
  NGFXFenceRef m_pFrameFence;
};

K3D_APP_MAIN(SwapchainPresent);

bool SwapchainPresent::OnInit() {
  App::OnInit();
#if !(K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_IOS)
  m_pRHI = ACQUIRE_PLUGIN(RHI_Vulkan);
  auto pVkRHI = k3d::StaticPointerCast<IVkRHI>(m_pRHI);
  pVkRHI->Initialize("SwapChainTest", true);
  pVkRHI->Start();
  m_pFactory = pVkRHI->GetFactory();
  DynArray<NGFXDeviceRef> Devices;
  m_pFactory->EnumDevices(Devices);
  m_pDevice = Devices[0];
#else
  m_pRHI = ACQUIRE_PLUGIN(RHI_Metal);
  auto pMtlRHI = k3d::StaticPointerCast<IMetalRHI>(m_pRHI);
  if (pMtlRHI) {
    pMtlRHI->Start();
    m_pDevice = pMtlRHI->GetPrimaryDevice();
  }
#endif
  InitRHIObjects();
  return true;
}

void SwapchainPresent::OnDestroy() {
  m_pRHI->Shutdown();
}

void SwapchainPresent::OnProcess(Message &msg) {
	auto cmdBuffer = m_pCmdQueue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT);
	auto presentImage = m_pSwapChain->GetCurrentTexture();

	cmdBuffer->Transition(presentImage, NGFX_RESOURCE_STATE_PRESENT);
	cmdBuffer->Present(m_pSwapChain, nullptr);
	cmdBuffer->Commit();
	cmdBuffer->Release();
}

void SwapchainPresent::InitRHIObjects()
{
	m_pCmdQueue = m_pDevice->CreateCommandQueue(NGFX_COMMAND_GRAPHICS);
	NGFXSwapChainDesc desc = { NGFX_PIXEL_FORMAT_RGBA8_UNORM, 1920, 1080, 2 };
	m_pSwapChain = m_pFactory->CreateSwapchain(m_pCmdQueue, HostWindow()->GetHandle(), desc);
  m_pFrameFence = m_pDevice->CreateFence();
}
