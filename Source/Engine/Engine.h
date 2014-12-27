#pragma once
#include <Config/Prerequisities.h>
#include <KTL/Singleton.h>
#include <Interface/IRenderer.h>

namespace k3d {
	class Engine {
	public:

		static Engine* CreateEngine();

		~Engine();

		void SetRenderer(IRenderer * renderer);

		void DoOnInitEngine();

		void DoOnDrawFrame();

	protected:
		Engine() = default;

	private:
		IRenderer *pRenderer;

	};
}
