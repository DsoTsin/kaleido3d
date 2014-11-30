#pragma once
#ifndef __DirectXRenderer_h__
#define __DirectXRenderer_h__

#include <KTL/NonCopyable.hpp>
#include <KTL/Singleton.hpp>
#include <KTL/RefCount.hpp>
#include <Interface/IRenderer.h>

#include <memory>
#include "DXCommon.h"

namespace k3d {
	
	class Shader;
	class Window;

	enum class DXFeature {
		Level_11_1,
		Level_11_2,
		Level_12_0
	};

	class DXDevice : public Singleton<DXDevice> 
	{
	public:

		~DXDevice();
		
		void Init(Window *, DXFeature feature);

		void Destroy();

		Ref<ID3D11Device>			Device() { return pDevice; }
		Ref<ID3D11DeviceContext>	ImmediateContext() { return pImmediateContext; }

		friend class DirectXRenderer;

		DXDevice();
	
	private:

		Ref<ID3D11Texture2D>			pBackBuffer;
		Ref<ID3D11Device> 				pDevice;
		Ref<IDXGISwapChain>				pSwapChain;
		Ref<ID3D11RenderTargetView>		pRenderTargetView;
		Ref<ID3D11DeviceContext>		pImmediateContext;
		Ref<ID3D11Texture2D>			pDepthStencil;
		Ref<ID3D11DepthStencilView>		pDepthStencilView;
	};

	class DirectXRenderer : public IRenderer, public Singleton<DirectXRenderer>, public std::enable_shared_from_this<DirectXRenderer>
	{
	public:

		~DirectXRenderer() override;

		void PrepareFrame() override;

		// traverse the scene tree and render the elements
		void DrawOneFrame() override;

		// do postprocessing and swap buffers
		void EndOneFrame() override;

		void DrawMesh(IRenderMesh *) override;
		void DrawMesh(IRenderMesh*, Shader*);
		void DrawMesh(IRenderMesh*, Shader*, Matrix4f const & matrix);

		void OnResize(int width, int height) override;

		DirectXRenderer() = default;
		
	protected:

		void SwapBuffers();
			
		bool isInitialized;

	};
}


#endif