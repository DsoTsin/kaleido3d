#include "DirectXRenderer.h"
#include "DXShader.h"
#include <assert.h>


using namespace DirectX;


namespace k3d {

	DirectXRenderer * DirectXRenderer::CreateRenderer(DXDevice * context) {
		DirectXRenderer * renderer = new DirectXRenderer(context);
		return renderer;
	}

	DirectXRenderer::DirectXRenderer(DXDevice * context) {
		pContext = context;

		isInitialized = false;
	}

	DirectXRenderer::~DirectXRenderer() {
		if (pContext)
			pContext->Destroy();
	}

	void DirectXRenderer::PrepareFrame() {
		pContext->pImmediateContext->ClearRenderTargetView(pContext->pRenderTargetView, Colors::MidnightBlue);
		pContext->pImmediateContext->ClearDepthStencilView(pContext->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
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

	void DirectXRenderer::OnResize(int width, int height) {

	}

	void DirectXRenderer::SwapBuffers()
	{
		assert(pContext != nullptr && pContext->pSwapChain != nullptr);
		pContext->pSwapChain->Present(0, 0);
	}

}