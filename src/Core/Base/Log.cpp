#include "CoreMinimal.h"
#include <cstdarg>
#include <mutex>

namespace k3d 
{
	void Log(ELogLevel const & Lv, const char * tag, const char * fmt, ...)
	{
		static /*thread_local*/ char dbgStr[2048] = { 0 };
		va_list va;
		va_start(va, fmt);
        Vsnprintf(dbgStr, 2048, fmt, va);
		va_end(va);

		auto logModule = StaticPointerCast<k3d::ILogModule>(GlobalModuleManager.FindModule("KawaLog"));
		if (logModule)
		{
			int logType = (int)ELoggerType::EConsole;
			switch (Lv)
			{
			case ELogLevel::Fatal:
			case ELogLevel::Error:
			case ELogLevel::Warn:
			case ELogLevel::Info:
				logType |= (int)ELoggerType::EWebsocket | (int)ELoggerType::EFile;
				break;
			case ELogLevel::Debug:
				logType |= (int)ELoggerType::EWebsocket;
				break;
			}
			if (logType & int(ELoggerType::EWebsocket))
			{
				k3d::ILogger* logger = logModule->GetLogger(ELoggerType::EWebsocket);
				if (logger)
				{
					logger->Log(Lv, tag, dbgStr);
				}
			}
			if (logType & int(ELoggerType::EConsole))
			{
				k3d::ILogger* logger = logModule->GetLogger(ELoggerType::EConsole);
				if (logger)
				{
					logger->Log(Lv, tag, dbgStr);
				}
			}
			if (logType & int(ELoggerType::EFile))
			{
				k3d::ILogger* logger = logModule->GetLogger(ELoggerType::EFile);
				if (logger)
				{
					logger->Log(Lv, tag, dbgStr);
				}
			}
		}
	}

}
