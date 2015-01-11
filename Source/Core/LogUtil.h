#pragma once
#ifndef __k3dDbg_h__
#define __k3dDbg_h__
#include <Config/Prerequisities.h>
#include "Utils/StringUtils.h"

namespace k3d {

	class Debug
	{
	public:
		Debug();
		Debug & operator << (const char *);
	};

	void kDebug(const char *fmt, ...);

	class Log
	{
		Log();
	public:
		static bool   InitLogFile(const char *name);
		static void   CloseLog();

		static void   Message(const char *format, ...);
		static void   Warning(const char *format, ...);
		static void   Error(const char *format, ...);
		static void   Fatal(const char *format, ...);
	};

#define LOG_WARN(expression, message) \
    if(!(expression)) Log::Warning(message);

#define LOG_MESSAGE(message) \
    Log::Message(message);

#define DBG_LINE_WITH_LAST_ERROR(message) \
	kDebug("%s in file(%s) line(%d) %s\n", (message), __FILE__, __LINE__, GetLastWin32Error().c_str());
}

#endif
