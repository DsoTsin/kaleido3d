#pragma once
#ifndef __LogUtil_h__
#define __LogUtil_h__
#include "Utils/StringUtils.h"

#include <KTL/Singleton.hpp>
#include <Interface/ILog.h>

#include "Os.h"

namespace k3d
{
	extern K3D_API void Log(ELogLevel const & Lv, const char* tag, const char *fmt, ...);
}

#ifndef K3DPLATFORM_OS_WIN
#define __debugbreak __builtin_trap
#endif

#define K3D_STRINGIFY(x) #x
#define K3D_STRINGIFY_BUILTIN(x) K3D_STRINGIFY(x)
#define K3D_ASSERT(isFalse, ...) \
	if (!(bool)(isFalse)) { \
		::k3d::Log(::k3d::ELogLevel::Error,"Assert","\nAssertion failed in " K3D_STRINGIFY_BUILTIN( __FILE__ ) " @ " K3D_STRINGIFY_BUILTIN( __LINE__ ) "\n"); \
		::k3d::Log(::k3d::ELogLevel::Error,"Assert","\'" #isFalse "\' is false"); \
		::k3d::Log(::k3d::ELogLevel::Error,"Assert", "args is" ##__VA_ARGS__); \
		__debugbreak(); \
    }


#define KLOG(Level, TAG, ...) \
	::k3d::Log(::k3d::ELogLevel::Level, #TAG, __VA_ARGS__);


#define DBG_LINE_WITH_LAST_ERROR(tag, message) \
	::k3d::Log(::k3d::ELogLevel::Error, (tag), "%s in file(%s) line(%d) %s", (message), __FILE__, __LINE__, GetLastWin32Error().c_str());

#endif
