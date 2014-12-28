#pragma once
#ifndef __DirectXRenderer_h__
#define __DirectXRenderer_h__

#include <Interface/IRenderer.h>
#include "DXCommon.h"

namespace k3d {
	 
	class Window;

	enum class DXFeature {
		Level_11_1,
		Level_11_2,
		Level_12_0
	};

	class DXDevice {
	public:

		~DXDevice();

		static DXDevice * CreateContext(Window * , DXFeature feature);

		void Destroy();

		ID3D11Device *			Device() { return pDevice; }
		ID3D11DeviceContext *	ImmediateContext() { return pImmediateContext; }

		friend class DirectXRenderer;
	private:

		DXDevice(Window *);

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

		void PrepareFrame() override;

		// traverse the scene tree and render the elements
		void DrawOneFrame() override;

		// do postprocessing and swap buffers
		void EndOneFrame() override;

		void DrawMesh(IRenderMesh *) override;

		void OnResize(int width, int height) override;

		static DirectXRenderer * CreateRenderer(DXDevice *);

	protected:

		void SwapBuffers();

		DirectXRenderer(DXDevice *);

		DirectXRenderer() = default;

		DXDevice * pContext;
	
		bool isInitialized;

	};
}


#endif