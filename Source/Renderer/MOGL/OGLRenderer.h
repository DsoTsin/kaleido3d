#pragma once
#include <Interface/IRenderer.h>

namespace k3d {
	
	class OGLRenderer : public IRenderer {
	public:
		~OGLRenderer() override;

	private:
		OGLRenderer();
	};
}