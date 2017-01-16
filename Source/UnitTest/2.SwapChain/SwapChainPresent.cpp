#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <Interface/IRHI.h>
#include <Renderer/Render.h>

using namespace k3d;
using namespace render;

rhi::GfxSetting setting{ 1920, 1080, rhi::EPF_RGBA8Unorm, rhi::EPF_D32Float, true, 2 };

class VkSwapChainPresent : public App
{
public:
	explicit VkSwapChainPresent(kString const & appName)
		: App(appName, 1920, 1080)
	{}
	VkSwapChainPresent(kString const & appName, uint32 width, uint32 height)
		: App(appName, width, height)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

private:
	RenderContext			m_RenderContext;

	rhi::DeviceRef			m_pDevice;
	rhi::RenderViewportRef	m_RenderVp;
};

K3D_APP_MAIN(VkSwapChainPresent);

bool VkSwapChainPresent::OnInit()
{
	App::OnInit();
#if K3DPLATFORM_OS_IOS || K3DPLATFORM_OS_MAC
    m_RenderContext.Init(RHIType::ERTMetal);
#else
    m_RenderContext.Init(RHIType::ERTVulkan);
#endif
	m_pDevice = m_RenderContext.GetDevice();
	m_RenderVp = m_pDevice->NewRenderViewport(HostWindow()->GetHandle(), setting);
	m_RenderVp->InitViewport(nullptr, nullptr, setting);
	return true;
}

void VkSwapChainPresent::OnDestroy()
{
	m_RenderContext.Destroy();
}

void VkSwapChainPresent::OnProcess(Message& msg)
{
	if (m_RenderVp)
	{
		m_RenderVp->PrepareNextFrame();
		m_RenderVp->Present(false);
	}
}
