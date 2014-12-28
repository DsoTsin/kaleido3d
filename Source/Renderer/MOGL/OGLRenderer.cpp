#include "OGLRenderer.h"

#include <Config/OSHeaders.h>
#include <GL/glew.h>
#include <assert.h>

namespace k3d {

	void OGLRenderer::PrepareFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OGLRenderer::DrawOneFrame()
	{
	}

	void OGLRenderer::EndOneFrame()
	{
		SwapBuffers();
	}

	void OGLRenderer::DrawMesh(IRenderMesh *)
	{
	}

	OGLRenderer * OGLRenderer::CreateRenderer(OGLDevice * device) {
		return new OGLRenderer(device);
	}

	OGLRenderer::~OGLRenderer() {
		if (pDevice != nullptr) {
			pDevice->Destroy();
			delete pDevice;
		}
	}


	OGLRenderer::OGLRenderer(OGLDevice * device)
	: pDevice(device) 
	{
	}

	void OGLRenderer::SwapBuffers() {
		assert(pDevice != nullptr);
		pDevice->SwapBuffers();
	}

	void OGLRenderer::OnResize(int width, int height) {

	}
}