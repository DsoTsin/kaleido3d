#ifndef __ILogModule_h__
#define __ILogModule_h__

#include <Core/Module.h>
#include <Core/Interface/ILog.h>

namespace k3d
{
	enum class ELoggerType : uint32
	{
		EFile = 1,
		EWebsocket = 2,
		EConsole = 4
	};

	class ILogModule : public IModule
	{
	public:
		virtual ~ILogModule() {}
		virtual ILogger* GetLogger(ELoggerType const&) = 0;
	};

}

#if	BUILD_STATIC_PLUGIN
K3D_STATIC_MODULE_DECLARE(KawaLog);
#endif
#endif // !__ILogModule_h__
