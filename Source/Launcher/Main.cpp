#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Core/AppBase.h>
#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>

using namespace k3d;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	AppBase * ptr = AppBase::CreateApplication(GetModuleHandle(NULL), NULL);

	Window window;
	window.SetWindowCaption("TsinStudio¹¤×÷ÊÒ");
	window.Resize(1900, 700);

	window.Show();

	IRenderer * renderer = RendererFactory::SetUpRenderer("opengl", &window);
	
	Engine * engine = Engine::CreateEngine();
	engine->SetRenderer(renderer);
	
	ptr->SetEngine(engine);
	ptr->StartMessageLooping();

	return 0;
}