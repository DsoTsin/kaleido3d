#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Core/AppBase.h>
#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>

#include <string>

using namespace k3d;
using namespace std;

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow) 
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);
	Log::InitLogFile("Launcher.html");

	wstring cmdLine{ lpCmdLine };
	string str(cmdLine.length(), ' ');
	if (!str.empty()) {
		std::copy(cmdLine.begin(), cmdLine.end(), str.begin());
		Log::Warning("cmdline %s", str.c_str());
	}
	else {
		str = "dx11";
	}

	std::unique_ptr<AppBase> & app = AppBase::CreateApplication(GetModuleHandle(NULL), NULL);

	Window window;
	window.SetWindowCaption("TsinStudio¹¤×÷ÊÒ");
	window.Resize(1900, 700);

	window.Show();

	IRenderer * renderer = RendererFactory::SetUpRenderer(str.c_str(), &window);
	
	Engine engine = Singleton<Engine>::Get();
	engine.SetRenderer(renderer);
	
	app->SetEngine(&engine);
	app->StartMessageLooping();

	Log::CloseLog();
	return 0;
}