#pragma once
#include "LogUtil.h"
#include "AssetManager.h"

namespace k3d {
    
    class App;
    
	namespace Core {

		extern App* Init(kString const & appName);
		
		extern void Log(
			LogLevel && level, 
			const char * tag, 
			const char *fmt, ...);

		extern ::k3d::AssetManager & GetAssetManager();

		extern kString GetExecutablePath();
	}
}