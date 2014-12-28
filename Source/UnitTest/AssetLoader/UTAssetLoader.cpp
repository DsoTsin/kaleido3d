#include <Config/OSHeaders.h>

#include <Core/LogUtil.h>
#include <Engine/AssetManager.h>


using namespace k3d;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	Log::InitLogFile("UTAssetLoader.html");
	
	AssetManager & assetManager = AssetManager::Get();
	assetManager.Init();


	assetManager.Shutdown();
	Log::CloseLog();
	return 0;
}