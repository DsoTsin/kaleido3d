#include "Kaleido3D.h"
#include "Module.h"
#include "App.h"
#include "Os.h"
#include "LogUtil.h"
#include <KTL/String.hpp>
#include <list>
#include <algorithm>
#include <utility>
#include <Config/OSHeaders.h>

namespace k3d
{
	typedef IModule * (*PFN_GetModule)();

	class ModuleManagerPrivate
	{
	public:
#if K3DPLATFORM_OS_WIN
		std::list<std::pair<IModule*, HMODULE> > g_ModuleList;
		std::unordered_map<std::string, HMODULE> g_Win32ModuleMap;
#endif
		std::unordered_map<String, ModuleRef> g_ModuleMap;
		mutable bool g_IsInited = false;
	};

	ModuleManager GlobalModuleManager;

	ModuleManager::~ModuleManager()
	{
		KLOG(Info, ModuleManager, "Destroyed");
		p->g_IsInited = false;
		if (!p->g_ModuleMap.empty())
		{
			p->g_ModuleMap.clear();
		}
		//KLOG(Info, ModuleManager, "Destroyed");

#if K3DPLATFORM_OS_WIN
		if (!p->g_Win32ModuleMap.empty())
		{
			for (auto & entry : p->g_Win32ModuleMap)
			{
				::FreeLibrary(entry.second);
			}
			p->g_Win32ModuleMap.clear();
		}
#endif
	}

	void ModuleManager::AddModule(const char * name, ModuleRef module)
	{
		if (!p->g_IsInited)
			return;
		p->g_ModuleMap[name] = module;
	}

	void ModuleManager::RemoveModule(const char * name)
	{
		if (!p->g_IsInited)
			return;
		p->g_ModuleMap.erase(name);
	}

	bool ModuleManager::LoadModule(const char * moduleName)
	{
		if (!p->g_IsInited)
			return nullptr;
		String entryFunction;
		entryFunction.AppendSprintf("Get%sModule", moduleName);
#if K3DPLATFORM_OS_WIN
        std::string moduleDir = "./";
		HMODULE hModule = LoadLibraryA((moduleDir + moduleName + ".dll").c_str());
		if (hModule)
		{
			PFN_GetModule pFn = (PFN_GetModule)::GetProcAddress((HMODULE)hModule, entryFunction.CStr());
			p->g_Win32ModuleMap[moduleName] = hModule;
			p->g_ModuleMap[moduleName] = ModuleRef(pFn());
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
			p->g_ModuleMap.insert({moduleName, ModuleRef(mod)});
			return true;
		}
#endif
		return false;
	}
	
	ModuleRef ModuleManager::FindModule(const char * moduleName)
	{
		if (!p->g_IsInited)
			return nullptr;
		if (p->g_ModuleMap.find(moduleName) != p->g_ModuleMap.end())
		{
			return p->g_ModuleMap[moduleName];
		} 
		else
		{
			if(LoadModule(moduleName))
				return p->g_ModuleMap[moduleName];
		}
		return nullptr;
	}

	ModuleManager::ModuleManager() : p(new ModuleManagerPrivate)
	{
		p->g_IsInited = true;
	}
}
