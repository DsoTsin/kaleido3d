#include "Kaleido3D.h"

#include "Renderer.h"

using namespace DirectX;

namespace k3d {

	DirectXRenderer::~DirectXRenderer() {
	}

	void DirectXRenderer::PrepareFrame() {
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
	}

}