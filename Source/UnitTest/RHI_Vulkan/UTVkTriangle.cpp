#include <Kaleido3D.h>
#include <Core/App.h>
#include <Core/Message.h>
#include <RHI/IRHI.h>
#include <Renderer/RenderContext.h>
#include "RHI/Vulkan/VkCommon.h"
#include "RHI/Vulkan/Public/VkRHI.h"
using namespace k3d;
class VkTriangleUnitTest : public App
{
public:
	explicit VkTriangleUnitTest(kString const & appName)
		: App(appName, 1920, 1080)
	{}

	bool OnInit() override;
	void OnDestroy() override;
	void OnProcess(Message & msg) override;

private:

	RenderContext			m_RenderContext;
	rhi::IDevice *			m_Device;
	rhi::IRenderViewport *	m_Viewport;
};

K3D_APP_MAIN(VkTriangleUnitTest);

bool VkTriangleUnitTest::OnInit()
{
	App::OnInit();
	m_RenderContext.Init(RHIType::ERTDirect3D12);
	return true;
}

void VkTriangleUnitTest::OnDestroy()
{

}

void VkTriangleUnitTest::OnProcess(Message& msg)
{

}