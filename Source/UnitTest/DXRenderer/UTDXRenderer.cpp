#include <Interface/IRenderer.h>
#include <Interface/IRenderMesh.h>

#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Core/AppBase.h>

#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>
#include <Renderer/DirectX/DirectXRenderer.h>

using namespace k3d;

int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
{
	Log::InitLogFile("UTDXRenderer.html");
	std::unique_ptr<AppBase> & app = AppBase::CreateApplication(GetModuleHandle(NULL), NULL);

	Window window;
	window.SetWindowCaption("UTDXRenderer");
	window.Resize(1700, 700);
	window.Show();

	DXDevice * context = DXDevice::CreateContext(&window, DXFeature::Level_11_2);
	IRenderer * renderer = DirectXRenderer::CreateRenderer(context);

	Engine engine = Singleton<Engine>::Get();
	engine.SetRenderer(renderer);

	app->SetEngine(&engine);
	app->StartMessageLooping();

	Log::CloseLog();
	return 0;
}