#pragma once
#ifndef __DirectXRenderer_h__
#define __DirectXRenderer_h__

#include <Interface/IRenderer.h>
#include <d3d11.h>

namespace k3d {
	 
	class Window;

	enum class DXFeature {
		Level_11_1,
		Level_11_2,
		Level_12_0
	};

	class DirectXContext {
	public:

		~DirectXContext();

		static DirectXContext * CreateContext(Window * , DXFeature feature);

		void Destroy();

		friend class DirectXRenderer;
	private:

		DirectXContext(Window *);

		ID3D11Device *				pDevice;
		IDXGISwapChain *			pSwapChain;
		ID3D11RenderTargetView *	pRenderTargetView;
		ID3D11DeviceContext *       pImmediateContext;
		ID3D11Texture2D *           pDepthStencil;
		ID3D11DepthStencilView *    pDepthStencilView;
	};

	class DirectXRenderer : public IRenderer {
	public:

		~DirectXRenderer() override;
		void SwapBuffers() override;

		static DirectXRenderer * CreateRenderer(DirectXContext *);

	protected:

		DirectXRenderer(DirectXContext *);

		DirectXRenderer() = default;

		DirectXContext * pContext;
	};
}


#endif