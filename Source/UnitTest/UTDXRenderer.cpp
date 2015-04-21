#include "Kaleido3D.h"
#include <Interface/IRenderer.h>
#include <Interface/IRenderMesh.h>

#include <Core>

#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>

#include <Renderer/DirectX/RenderMesh.h>
#include <Renderer/DirectX/Device.h>
#include <Renderer/DirectX/Renderer.h>

using namespace k3d;

// 跨线程通信 InterThreadCommunication
SpConditionVariable			gTestMeshReady;
std::vector<IRenderMesh*>	gTestRenderMeshes;
std::vector<SpMesh>			gTestMeshes;

int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
{
	Log::InitLogFile("UTDXRenderer.html");
	AssetManager::Get().Init();
//	TaskManager::Get().Init();
//	TaskManager::Get().Post(new TestMesh("Test/TestDCC_1.kspack"));

	Window window("UTDXRenderer", 1700, 700);
	window.Show();

	d3d12::Device::Get().Init(&window);
	
	DirectXRenderer & renderer = DirectXRenderer::Get();

	Message msg;
	while (window.IsOpen())
	{
		bool isQuit = false;
		while (window.PollMessage(msg))
		{
			if (msg.type == Message::MouseButtonPressed)
			{
				Debug::Out("Main", "leftbutton");
			}
			else if (msg.type == Message::Closed)
			{
				isQuit = true;
				break;
			}
		}

		renderer.PrepareFrame();
			renderer.DrawOneFrame();

			//
			//
			//
		renderer.EndOneFrame();

		if (isQuit)
			break;
	}

	Log::CloseLog();
	return 0;
}