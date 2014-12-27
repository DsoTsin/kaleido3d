#include "Engine.h"
namespace k3d {

	Engine * Engine::CreateEngine()
	{
		Engine *engine = new Engine;
		engine->pRenderer = nullptr;
		return engine;
	}

	Engine::~Engine()
	{
		if (pRenderer != nullptr) {
			delete pRenderer;
			pRenderer = nullptr;
		}
	}

	void Engine::SetRenderer(IRenderer * renderer)
	{
		pRenderer = renderer;
	}

	void Engine::DoOnInitEngine()
	{
	}

	void Engine::DoOnDrawFrame()
	{
		assert(pRenderer != nullptr && "You should call \"SetRenderer\" method!!");
		pRenderer->SwapBuffers();
	}



}