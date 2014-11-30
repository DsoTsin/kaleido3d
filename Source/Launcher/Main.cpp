#include <Core/LogUtil.h>
#include <Core/Window.h>
#include <Windows.h>
#include <Renderer/DirectX/DirectXRenderer.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	k3d::Window window;
	window.SetWindowCaption("TsinStudio¹¤×÷ÊÒ");
	window.Resize(1900, 700);

	k3d::DirectXContext *context = k3d::DirectXContext::CreateContext(&window, k3d::DXFeature::Level_11_2);
	k3d::DirectXRenderer *renderer = k3d::DirectXRenderer::CreateRenderer(context);
	
	window.Show();

	return 0;
}