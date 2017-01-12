#include "Kaleido3D.h"
#include "Module.h"
#include "App.h"
#include "Os.h"
#include <list>
#include <algorithm>
#include <utility>
#include <Config/OSHeaders.h>

namespace k3d
{
	typedef IModule * (*PFN_GetModule)();

#if K3DPLATFORM_OS_WIN
	std::list<std::pair<IModule*, HMODULE> > g_ModuleList;
#endif

	ModuleManager GlobalModuleManager;
	static std::unordered_map<std::string, IModule*> g_ModuleMap;

#if K3DPLATFORM_OS_WIN
	static std::unordered_map<std::string, HMODULE> g_Win32ModuleMap;
#endif

	ModuleManager::~ModuleManager()
	{
		//g_ModuleMap.clear(); TODO crash
	}

	void ModuleManager::AddModule(const char * name, IModule * module)
	{
		g_ModuleMap[name] = module;
	}

	void ModuleManager::RemoveModule(const char * name)
	{
		g_ModuleMap.erase(name);
	}

	bool ModuleManager::LoadModule(const char * moduleName)
	{
#if K3DPLATFORM_OS_WIN
        std::string moduleDir = "./";
		HMODULE hModule = LoadLibraryA((moduleDir + moduleName + ".dll").c_str());
		if (hModule)
		{
			std::string entryName("Get");
			entryName += moduleName;
			entryName += "Module";
			PFN_GetModule pFn = (PFN_GetModule)::GetProcAddress((HMODULE)hModule, entryName.c_str());
			g_Win32ModuleMap[moduleName] = hModule;
			g_ModuleMap[moduleName] = pFn();
			return true;
		}
#else
		std::string entryName("Get");
		entryName += moduleName;
		entryName += "Module";
        kString libDir = GetEnv()->GetEnvValue(Environment::ENV_KEY_MODULE_DIR) + "/lib" + moduleName +
#if K3DPLATFORM_OS_MAC
        ".dylib";
#else
        ".so";
#endif
		void* handle = ::dlopen(libDir.c_str(), RTLD_LAZY);
		if(handle) {
			PFN_GetModule fn = (PFN_GetModule)dlsym(handle, entryName.c_str());
			if(!fn) {
				return false;
			}
			g_ModuleMap[moduleName] = fn();
			return true;
		}
#endif
		return false;
	}
	
	IModule * ModuleManager::FindModule(const char * moduleName)
	{
		std::string mName(moduleName);
		if (g_ModuleMap.find(moduleName) != g_ModuleMap.end())
		{
			return g_ModuleMap[mName];
		} 
		else
		{
			if(LoadModule(moduleName))
				return g_ModuleMap[mName];
		}
		return nullptr;
	}

	ModuleManager::ModuleManager()
	{
	}
}
