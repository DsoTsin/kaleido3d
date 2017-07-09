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
	string cmdLine{ lpCmdLine };
	if (!cmdLine.empty()) {
	}
	else {
		cmdLine = "dx11";
	}

	Window window;
	window.SetWindowCaption("TsinStudio¹¤×÷ÊÒ");
	window.Resize(1900, 700);

	window.Show();

	return 0;
}