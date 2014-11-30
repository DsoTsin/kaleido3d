#include "Kaleido3D.h"

#include <Core>

#include <Renderer/DirectX/RenderMesh.h>
#include <Renderer/DirectX/DeviceManager.h>
#include "SampleRenderer.h"

#include <Renderer/DirectX/RootSignature.h>
#include <Renderer/DirectX/PipeLineState.h>
#include <Core/App.h>
#include <Core/ModuleCore.h>

using namespace k3d;
using namespace k3d::d3d12;

/*
int main(int argc, const char ** argv)
{
	Core::Init(L"SampleRenderer");
	Window window("SampleRenderer", 1700, 800);
	window.Show();

	gD3DDevice->CreateDeviceResources();
	gD3DDevice->SetWindows((HWND)window.GetHandle(), 1700, 900);
	
	SampleRenderer & renderer = SampleRenderer();
	renderer.Initialize();

	Message msg;
	while (window.IsOpen())
	{
		bool isQuit = false;
		while (window.PollMessage(msg))
		{
			if (msg.type == Message::MouseButtonPressed)
			{
				Log::Out("Main", "leftbutton");
			}
			else if (msg.type == Message::Closed)
			{
				isQuit = true;
				break;
			}
		}
		renderer.Render(nullptr);
		gD3DDevice->Present();
		if (isQuit)
			break;
	}

	return 0;
}
*/

class SampleApp : public App {
public:
	SampleApp(const kchar * appName) :
		App(appName)
	{}
};

K3D_APP_MAIN(SampleApp);