#ifndef __Module_h__
#define __Module_h__

#if K3DPLATFORM_OS_WIN
#define PLUGIN_API_DECLARE  __declspec(dllexport)
#else
#define PLUGIN_API_DECLARE  
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
class MoudleName##StaticInitializer {\
public:\
	MoudleName##StaticInitializer() { k3d::ModuleManager::Get().AddModule(#ModuleName, new MoudleClass); }\
	~MoudleName##StaticInitializer() { k3d::ModuleManager::Get().RemoveModule(#ModuleName);	}\
}; \
static MoudleName##StaticInitializer mInitializer;

/**
 * build dlls
 */
#ifdef BUILD_SHARED_CORE
#define MODULE_IMPLEMENT K3D_DYNAMIC_MODULE_IMPLEMENT
#else
#define MODULE_IMPLEMENT K3D_STATIC_MODULE_IMPLEMENT
#endif

namespace k3d
{
	class IModule
	{
	public:
		virtual void Start() = 0;
		virtual void Shutdown() = 0;
		virtual const char * Name() = 0;
		virtual ~IModule() {}
	};

	class CORE_API ModuleManager
	{
	public:
		~ModuleManager();

		void AddModule(const char* name, IModule* module);
		void RemoveModule(const char * name);
		bool LoadModule(const char * moduleName);
		IModule* FindModule(const char * moduleName);

		ModuleManager();
	};

	extern CORE_API ModuleManager GlobalModuleManager;
}

#endif
