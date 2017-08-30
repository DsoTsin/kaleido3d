#ifndef __Module_h__
#define __Module_h__

namespace k3d
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

	class ModuleManagerPrivate;

	class K3D_CORE_API ModuleManager
	{
	public:
		~ModuleManager();

		void AddModule(const char* name, ModuleRef module);
		void RemoveModule(const char * name);
		bool LoadModule(const char * moduleName);
		ModuleRef FindModule(const char * moduleName);

		ModuleManager();

	private:
		ModuleManagerPrivate * p;
	};

	extern K3D_CORE_API ModuleManager GlobalModuleManager;
}

#endif
