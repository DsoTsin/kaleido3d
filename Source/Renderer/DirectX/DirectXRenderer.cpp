#include "DirectXRenderer.h"

using namespace k3d;

DirectXRenderer * DirectXRenderer::CreateRenderer(DirectXContext * context) {
	DirectXRenderer * renderer = new DirectXRenderer(context);
	return renderer;
}

DirectXRenderer::DirectXRenderer(DirectXContext * context) {
	(void)context;

}

DirectXRenderer::~DirectXRenderer() {

}