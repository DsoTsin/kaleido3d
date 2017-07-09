#ifndef __Module_h__
#define __Module_h__

#include <Interface/IModule.h>

namespace k3d
{
	class ModuleManagerPrivate;

	class CORE_API ModuleManager
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


	extern CORE_API ModuleManager GlobalModuleManager;
}

#endif
