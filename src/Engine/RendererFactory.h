#pragma once

namespace k3d {

	class IWindow;

	class RendererFactory {
	public:
		
		~RendererFactory();

		/*static IRenderer * SetUpRenderer(const char *, IWindow *);*/

	private:
		RendererFactory();
		
	};

}