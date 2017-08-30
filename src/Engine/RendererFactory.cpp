#include "Kaleido3D.h"
#include "RendererFactory.h"
#include <Core/Window.h>

using namespace std;
namespace k3d {
	/*
	static map<string, IRenderer*> gRendererMap;
	IRenderer * RendererFactory::SetUpRenderer(const char *name, IWindow *window) {
		assert(window != nullptr);
		string renderName{ name };
		IRenderer *renderer = nullptr;
		if (gRendererMap.find(renderName) == gRendererMap.end()) {
			if (renderName == "opengl") {
				OGLDevice * device = OGLDevice::CreateGLDevice(window, {GLDeviceFormat::RGBA, 32, 0, 16});
				renderer = OGLRenderer::CreateRenderer(device);
			}
			else if (renderName == "dx11") {
				DXDevice * context = DXDevice::CreateContext(window, DXFeature::Level_11_2);
				renderer = DirectXRenderer::CreateRenderer(context);
			}
			else {
				return renderer;
			}
			gRendererMap[renderName] = renderer;
		}
		else {
			return gRendererMap[renderName];
		}
		
		return renderer;
	}
	*/
	RendererFactory::RendererFactory() {

	}

	RendererFactory::~RendererFactory() {

	}

}