#include "Kaleido3D.h"

#include <Core>

#include <Renderer/DirectX/RenderMesh.h>
#include <Renderer/DirectX/Renderer.h>

#include <Renderer/DirectX/Wrapper.h>
#include <Renderer/DirectX/Infrastructure/Shader.h>
#include <Renderer/DirectX/Infrastructure/VertexBuffer.h>
#include <Renderer/DirectX/RootSignature.h>
#include <Renderer/DirectX/PipeLineState.h>

using namespace k3d;
using namespace k3d::d3d12;


int main(int argc, const char ** argv)
{
	AssetManager::Get().Init();
	Window window("UTDXRenderer", 1700, 700);
	window.Show();

	XRenderer & renderer = XRenderer();
	
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


		if (isQuit)
			break;
	}

	return 0;
}