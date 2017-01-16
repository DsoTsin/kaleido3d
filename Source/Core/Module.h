#ifndef __Module_h__
#define __Module_h__

#include <Interface/IModule.h>

namespace k3d
{
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
