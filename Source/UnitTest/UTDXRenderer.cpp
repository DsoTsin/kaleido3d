#include <Interface/IRenderer.h>
#include <Interface/IRenderMesh.h>

#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Core/Message.h>
#include <Core/AsynMeshTask.h>
#include <Core/ConditionVariable.h>

#include <Engine/Engine.h>
#include <Engine/RendererFactory.h>

#include <Renderer/DirectX/DirectXRenderer.h>

using namespace k3d;

// 跨线程通信 InterThreadCommunication
SpConditionVariable gTestMeshReady;

class TestMesh : public AsynMeshTask {
public:

	TestMesh(const char * meshPath) : AsynMeshTask(meshPath)
	{

	}

	void OnFinish() override
	{
		gTestMeshReady = SpConditionVariable(new ConditionVariable());
		gTestMeshReady->Signal();
	}
};


int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
{
	Log::InitLogFile("UTDXRenderer.html");

	Window window("UTDXRenderer", 1700, 700);
	window.Show();

	DXDevice * context = DXDevice::CreateContext(&window, DXFeature::Level_11_2);
	IRenderer * renderer = DirectXRenderer::CreateRenderer(context);

	Engine engine = Singleton<Engine>::Get();
	engine.SetRenderer(renderer);

	Message msg;
	while (window.PollMessage(msg))
	{

	}

	Log::CloseLog();
	return 0;
}