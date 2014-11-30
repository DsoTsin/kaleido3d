#pragma once
#include "Common.h"
#include <Interface/IRenderer.h>
//#include <Interface/IRenderable.h>

NS_K3D_METAL_BEGIN

class Renderable;

class Renderer {
public:
    Renderer();
    
    void Render(Renderable * r);
};

NS_K3D_METAL_END