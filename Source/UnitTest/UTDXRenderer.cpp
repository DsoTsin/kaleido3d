#include "Kaleido3D.h"
#include <Interface/IRenderer.h>
#include <Interface/IRenderMesh.h>

#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Core/Message.h>
#include <Core/AsynMeshTask.h>
#include <Core/ConditionVariable.h>
#include <Core/AssetManager.h>

#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>

#include <Renderer/DirectX/DXRenderMesh.h>
#include <Renderer/DirectX/DirectXRenderer.h>

using namespace k3d;

// 跨线程通信 InterThreadCommunication
SpConditionVariable			gTestMeshReady;
std::vector<IRenderMesh*>	gTestRenderMeshes;
std::vector<SpMesh>			gTestMeshes;

class TestMesh : public AsynMeshTask {
public:

	TestMesh(const char * meshPath) : AsynMeshTask(meshPath)
	{

	}

	void OnFinish() override
	{
		gTestMeshReady = SpConditionVariable(new ConditionVariable());
		gTestMeshReady->Signal();
		gTestMeshes = m_MeshPtrList;
	}
};


int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
{
	Log::InitLogFile("UTDXRenderer.html");
	AssetManager::Get().Init();
	TaskManager::Get().Init();
	TaskManager::Get().Post(new TestMesh("Test/TestDCC_1.kspack"));

	Window window("UTDXRenderer", 1700, 700);
	window.Show();

	DXDevice::Get().Init(&window, DXFeature::Level_11_2);
	
	DirectXRenderer & renderer = DirectXRenderer::Get();

	std::shared_ptr<DXVertexShader>				gVertexShader;
	std::shared_ptr<DXPixelShader>				gPixelShader;

	gVertexShader = std::shared_ptr<DXVertexShader>(new DXVertexShader("Test/ForwardLight.hlsl", "RenderSceneVS", "vs_5_0"));
	gVertexShader->Init();

	gPixelShader = std::shared_ptr<DXPixelShader>(new DXPixelShader("Test/ForwardLight.hlsl", "AmbientLightPS", "ps_5_0"));
	gPixelShader->Init();

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

			if (gTestRenderMeshes.empty()) 
			{
				ConditionVariable::WaitFor(gTestMeshReady);
				Debug::Out("wWinMain","TestMesh(%d) is Ready\n", gTestMeshes.size());
				for (auto mesh : gTestMeshes)
				{
					DXRenderMesh * rMesh = new DXRenderMesh;
					rMesh->Init(DXDevice::Get(), mesh, *gVertexShader);
					gTestRenderMeshes.push_back(rMesh);
				}
			} 
			else
			{
				for (auto mesh : gTestRenderMeshes) 
				{
					renderer.DrawMesh(mesh, nullptr, Matrix4f());
				}
			}
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