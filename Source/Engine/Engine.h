#pragma once
#include <KTL/Singleton.hpp>

namespace k3d {
	class Engine {
	public:
		
		~Engine();

		void DoOnInitEngine();

		void DoOnDrawFrame();
		// trick
		friend class Singleton<Engine>;
	protected:
		Engine();

	private:
	
	};
}
