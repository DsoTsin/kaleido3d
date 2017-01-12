#include "VkCommon.h"
#include "VkRHI.h"
#include <Core/Module.h>
#include <Log/Public/ILogModule.h>
#include <cstdarg>
#include <Core/WebSocket.h>

PFN_vklogCallBack g_logCallBack = nullptr;

static thread_local char logContent[2048] = { 0 };
#define LOG_SERVER 1

#if K3DPLATFORM_OS_ANDROID
#define OutputDebugStringA(str) __android_log_print(ANDROID_LOG_DEBUG, "VkRHI", "%s", str);
#endif

void VkLog(k3d::ELogLevel const& Lv, const char * tag, const char * fmt, ...) 
{
	if (g_logCallBack != nullptr)
	{
		va_list va;
		va_start(va, fmt);
		g_logCallBack(Lv, tag, fmt, va);
		va_end(va);
	}
	else
	{
#if LOG_SERVER
		va_list va;
		va_start(va, fmt);
		vsprintf(logContent, fmt, va);
		va_end(va);

		k3d::ILogModule* logModule = (k3d::ILogModule*)k3d::GlobalModuleManager.FindModule("KawaLog");
		if (logModule)
		{
			k3d::ILogger* logger = logModule->GetLogger(k3d::ELoggerType::EWebsocket);
			if (logger)
			{
				logger->Log(Lv, tag, logContent);
			}

			if (Lv >= k3d::ELogLevel::Debug)
			{
				logger = logModule->GetLogger(k3d::ELoggerType::EConsole);
				if (logger)
				{
					logger->Log(Lv, tag, logContent);
				}
			}

			if (Lv >= k3d::ELogLevel::Warn)
			{
				logger = logModule->GetLogger(k3d::ELoggerType::EFile);
				if (logger)
				{
					logger->Log(Lv, tag, logContent);
				}
			}
		}
#else
		va_list va;
		va_start(va, fmt);
		vsprintf(logContent, fmt, va);
		va_end(va);
		OutputDebugStringA(logContent);
#endif
	}
}

void SetVkLogCallback(PFN_vklogCallBack func)
{
	g_logCallBack = func;
}