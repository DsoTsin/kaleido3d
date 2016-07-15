#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/File.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
#include <Renderer/RenderContext.h>
#include <Renderer/Render.h>
#include "RHI/Vulkan/VkCommon.h"
#include "RHI/Vulkan/Public/VkRHI.h"

using namespace k3d;
using namespace render;

rhi::GfxSetting setting{ 1920, 1080, rhi::EPF_RGBA8Unorm, rhi::EPF_RGBA8Unorm, false, 2 };

class VkSwapChainPresent : public App
{
public:
	explicit VkSwapChainPresent(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

private:
	RenderContext			m_RenderContext;

	rhi::IDevice *			m_pDevice;
	rhi::IRenderViewport *	m_RenderVp;
};

K3D_APP_MAIN(VkSwapChainPresent);

bool VkSwapChainPresent::OnInit()
{
	App::OnInit();
	m_RenderContext.Init(RHIType::ERTVulkan);
	m_pDevice = m_RenderContext.GetDevice();
	m_RenderVp = m_pDevice->NewRenderViewport(HostWindow()->GetHandle(), setting);
	m_RenderVp->InitViewport(nullptr, nullptr, setting);



	return true;
}

void VkSwapChainPresent::OnDestroy()
{
	if (m_RenderVp)
	{
		delete m_RenderVp;
		m_RenderVp = nullptr;
	}
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