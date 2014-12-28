#include "Engine.h"
namespace k3d {

	Engine::Engine()
	{
		pRenderer = nullptr;
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
		pRenderer->PrepareFrame();
		//....
		pRenderer->DrawOneFrame();

		//....
		pRenderer->EndOneFrame();
	}



}