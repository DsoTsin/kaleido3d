#include "DirectXRenderer.h"

#include <assert.h>

namespace k3d {

	DirectXRenderer * DirectXRenderer::CreateRenderer(DirectXContext * context) {
		DirectXRenderer * renderer = new DirectXRenderer(context);
		return renderer;
	}

	DirectXRenderer::DirectXRenderer(DirectXContext * context) {
		pContext = context;
	}

	DirectXRenderer::~DirectXRenderer() {

	}

	void DirectXRenderer::SwapBuffers()
	{
		assert(pContext != nullptr && pContext->pSwapChain != nullptr);
		pContext->pSwapChain->Present(0, 0);
	}

}