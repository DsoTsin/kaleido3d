#include "rendererwidget.h"
#include <QTimer>
#include <QResizeEvent>
#include <Core/LogUtil.h>

using namespace k3d;

RendererWidget::RendererWidget(QWidget *parent)
  : QWidget(parent)
  , Timer(nullptr)
{
  Timer = new QTimer;
  Timer->setInterval(16);
  Timer->setTimerType(Qt::PreciseTimer);
}

RendererWidget::~RendererWidget()
{
  if (Timer)
  {
    Timer->stop();
    delete Timer;
    Timer = nullptr;
  }
}

void RendererWidget::init()
{
  RHI = StaticPointerCast<IVkRHI>(ACQUIRE_PLUGIN(RHI_Vulkan));
  RHI->Initialize("Widget", false);
  RHI->Start();
  auto pFactory = RHI->GetFactory();
  DynArray<NGFXDeviceRef> Devices;
  pFactory->EnumDevices(Devices);
  Device = Devices[0];

  Queue = Device->CreateCommandQueue(NGFX_COMMAND_GRAPHICS);

  k3d::SwapChainDesc Desc = { 
    NGFX_PIXEL_FORMAT_RGBA8_UNORM_SRGB,
    width(),
    height(),
    2 };
  Swapchain = pFactory->CreateSwapchain(Queue, (void*)winId(), Desc);

  FrameFence = Device->CreateFence();

  QObject::connect(Timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
  Timer->start();
}

void RendererWidget::resizeEvent(QResizeEvent * pEvent)
{
  QWidget::resizeEvent(pEvent);
  auto size = pEvent->size();
  KLOG(Info, RendererWidget, "Resized %d, %d", size.width(), size.height());
  Swapchain->Resize(size.width(), size.height());
}

void RendererWidget::tickRender()
{
  auto cmdBuffer = Queue->ObtainCommandBuffer(NGFX_COMMAND_USAGE_ONE_SHOT);
  auto presentImage = Swapchain->GetCurrentTexture();
  cmdBuffer->Transition(presentImage, NGFX_RESOURCE_STATE_PRESENT);
  cmdBuffer->Present(Swapchain, nullptr);
  cmdBuffer->Commit(FrameFence);
}

void RendererWidget::onTimeOut()
{
  tickRender();
}