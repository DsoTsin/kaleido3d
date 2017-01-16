#include "Kaleido3D.h"
#include "Module.h"
#include "App.h"
#include "Os.h"
#include <KTL/String.hpp>
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
	static std::unordered_map<String, IModule*> g_ModuleMap;

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
		String entryFunction;
		entryFunction.AppendSprintf("Get%sModule", moduleName);
#if K3DPLATFORM_OS_WIN
        std::string moduleDir = "./";
		HMODULE hModule = LoadLibraryA((moduleDir + moduleName + ".dll").c_str());
		if (hModule)
		{
			PFN_GetModule pFn = (PFN_GetModule)::GetProcAddress((HMODULE)hModule, entryFunction.CStr());
			g_Win32ModuleMap[moduleName] = hModule;
			g_ModuleMap[moduleName] = pFn();
			return true;
		}
#else

#if !K3DPLATFORM_OS_IOS
        kString libDir = GetEnv()->GetEnvValue(Environment::ENV_KEY_MODULE_DIR) + "/lib" + moduleName +
#if K3DPLATFORM_OS_MAC
        ".dylib";
#else
        ".so";
#endif
        if(libDir.empty())
        {
            return false;
        }
        void* handle = ::dlopen(libDir.c_str(), RTLD_LAZY);
#else
        //kString libDir = kString(moduleName) + ".framework/" + moduleName;
        String libDir;
        libDir.AppendSprintf("%s.framework/%s", moduleName, moduleName);
        void* handle = ::dlopen(libDir.CStr(), RTLD_LAZY);
#endif
		if(handle)
        {
			PFN_GetModule fn = (PFN_GetModule)dlsym(handle, entryFunction.CStr());
			if(!fn)
            {
				return false;
			}
            auto mod = fn();
			//g_ModuleMap[moduleName] = mod;
            g_ModuleMap.insert({moduleName, mod});
			return true;
		}
#endif
		return false;
	}
	
	IModule * ModuleManager::FindModule(const char * moduleName)
	{
		if (g_ModuleMap.find(moduleName) != g_ModuleMap.end())
		{
			return g_ModuleMap[moduleName];
		} 
		else
		{
			if(LoadModule(moduleName))
				return g_ModuleMap[moduleName];
		}
		return nullptr;
	}

	ModuleManager::ModuleManager()
	{
	}
}
