#include "Kaleido3D.h"
#include "DirectXRenderer.h"
#include "DirectXRenderer.h"
#include "DXShader.h"
#include <assert.h>


using namespace DirectX;


namespace k3d {

	DirectXRenderer::~DirectXRenderer() {
		DXDevice::Get().Destroy();
	}

	void DirectXRenderer::PrepareFrame() {
		DXDevice::Get().pImmediateContext->ClearRenderTargetView(DXDevice::Get().pRenderTargetView, Colors::MidnightBlue);
		DXDevice::Get().pImmediateContext->ClearDepthStencilView(DXDevice::Get().pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		if ( !isInitialized ) {
			isInitialized = true;
		}
	}

	void DirectXRenderer::DrawOneFrame() {
		if (!isInitialized)
			return;
	}

	void DirectXRenderer::EndOneFrame() {
		this->SwapBuffers();
	}

	void DirectXRenderer::DrawMesh(IRenderMesh *) {

	}

	void DirectXRenderer::DrawMesh(IRenderMesh *, Shader *)
	{
	}

	void DirectXRenderer::DrawMesh(IRenderMesh *, Shader *, Matrix4f const & matrix)
	{

	}
	
	void DirectXRenderer::OnResize(int width, int height) {

	}

	void DirectXRenderer::SwapBuffers()
	{
		DXDevice::Get().pSwapChain->Present(0, 0);
	}

}