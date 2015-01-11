#include <Config/OSHeaders.h>

#include <Core/LogUtil.h>
#include <Core/TaskManager.h>
#include <Core/AsynMeshTask.h>
#include <Core/AssetManager.h>
#include <string>

using namespace k3d;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	Log::InitLogFile("UTAssetLoader.html");
	
	AssetManager & assetManager = AssetManager::Get();
	assetManager.Init();

	const char * pathPrefix = ::getenv("Kaleido3d_Dir");

	for (int i = 1; i < 4; i++) {
		char buffer[128] = { 0 };
		sprintf(buffer, "%s\\Test\\TestDCC_%d.kspack", pathPrefix, i);
		AsynMeshTask * meshTask = new AsynMeshTask(buffer);
		TaskManager::Get().Post(meshTask);
	}
	assetManager.Shutdown();
	Log::CloseLog();
	return 0;
}