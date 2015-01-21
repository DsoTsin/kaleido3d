#include <Config/OSHeaders.h>
#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>

#include <string>

using namespace k3d;
using namespace std;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) 
{
	Log::InitLogFile("Launcher.html");

	string cmdLine{ lpCmdLine };
	if (!cmdLine.empty()) {
		Log::Warning("cmdline %s", cmdLine.c_str());
	}
	else {
		cmdLine = "dx11";
	}

	Window window;
	window.SetWindowCaption("TsinStudio¹¤×÷ÊÒ");
	window.Resize(1900, 700);

	window.Show();
	/*
	IRenderer * renderer = RendererFactory::SetUpRenderer(cmdLine.c_str(), &window);
	
	Engine engine = Singleton<Engine>::Get();
	engine.SetRenderer(renderer);
	*/


	Log::CloseLog();
	return 0;
}