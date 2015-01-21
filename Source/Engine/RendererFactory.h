#pragma once

#include <Interface/IRenderer.h>

namespace k3d {

	class Window;

	class RendererFactory {
	public:
		
		~RendererFactory();

		/*static IRenderer * SetUpRenderer(const char *, Window *);*/

	private:
		RendererFactory();
		
	};

}