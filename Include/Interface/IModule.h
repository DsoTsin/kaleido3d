#pragma once
#include "Config/Config.h"
#include <KTL/SharedPtr.hpp>

#if K3DPLATFORM_OS_WIN
#define PLUGIN_API_DECLARE  __declspec(dllexport)
#else
#define PLUGIN_API_DECLARE  __attribute__((visibility("default")))
#endif

#ifdef BUILD_SHARED_LIB
#ifdef BUILD_WITH_PLUGIN
#if K3DPLATFORM_OS_WIN
#define CORE_API __declspec(dllimport)
#else
#define CORE_API __attribute__((visibility("default")))
#endif
#else
#if K3DPLATFORM_OS_WIN
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __attribute__((visibility("default")))
#endif
#endif
#else
#define CORE_API
#endif

#define K3D_DYNAMIC_MODULE_IMPLEMENT(ModuleName, ModuleClass) \
extern "C" PLUGIN_API_DECLARE ::k3d::IModule *Get##ModuleName##Module() { return new ModuleClass; }

#define K3D_STATIC_MODULE_IMPLEMENT(ModuleName, MoudleClass) \
extern "C" ::k3d::IModule *Get##ModuleName##Module() { \
return new MoudleClass; \
}
#define K3D_STATIC_MODULE_DECLARE(ModuleName) \
extern "C" ::k3d::IModule *Get##ModuleName##Module()

/**
* build dlls
*/
#ifdef BUILD_SHARED_CORE
#define MODULE_IMPLEMENT(ModuleName, MoudleClass) K3D_DYNAMIC_MODULE_IMPLEMENT(ModuleName, MoudleClass)
#else
#define MODULE_IMPLEMENT(ModuleName, MoudleClass) K3D_STATIC_MODULE_IMPLEMENT(ModuleName, MoudleClass)
#endif

#if BUILD_STATIC_PLUGIN
#define ACQUIRE_PLUGIN(PLUGIN) Get##PLUGIN##Module();
#else
#define ACQUIRE_PLUGIN(PLUGIN) k3d::GlobalModuleManager.FindModule(#PLUGIN)
#endif

K3D_COMMON_NS
{
	/**
	 * Module interface definition [RHI, ShaderCompiler, etc]
	 * @see ModuleManager [Core/ModuleManager]
	 */
	class IModule
	{
	public:
		virtual void Start() = 0;
		virtual void Shutdown() = 0;
		virtual const char * Name() = 0;
		virtual ~IModule() {}
	};

	typedef SharedPtr<IModule> ModuleRef;
}