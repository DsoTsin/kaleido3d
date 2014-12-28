#pragma once
#include <Config/Prerequisities.h>
#include <KTL/Singleton.h>
#include <Interface/IRenderer.h>

namespace k3d {
	class Engine {
	public:
		
		~Engine();

		void SetRenderer(IRenderer * renderer);

		void DoOnInitEngine();

		void DoOnDrawFrame();
		// trick
		friend class Singleton<Engine>;
	protected:
		Engine();

	private:
		IRenderer *pRenderer;

	};
}
