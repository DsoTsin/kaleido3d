#pragma once
#ifndef __k3dDbg_h__
#define __k3dDbg_h__
#include "Utils/StringUtils.h"
#include <KTL/Singleton.hpp>
#include <mutex>

struct IIODevice;

namespace k3d {

	enum class LogLevel
	{
		Debug,
		Info, /** [Message] */
		Warn, /** [Warning] */
		Error,/** [Error]*/
		Fatal /** [Fatal]*/
	};

	class Log : public Singleton<Log>
	{
	public:
		Log & operator << (const char *);
		Log & operator << (std::string const & str);
		
		void LogOutPut(LogLevel lev, const char * buffer);
		void Destroy();

	public:
		static void Out(const char * tag, const char *fmt, ...);
		static void Out(const char * tag, std::string const & log);
		static void Out(LogLevel && log, const char * tag, const char *fmt, ...);
        
        typedef void (*OutPutCallBack)(const char *);
        static void SetDebugOutFunction(OutPutCallBack callBack);

	private:
		std::mutex				s_LogLock;
		IIODevice *				s_LogFile;
		friend class					LogUtilInitializer;

	protected:
		Log();
		friend class Singleton<Log>;
	};


#define DBG_LINE_WITH_LAST_ERROR(tag, message) \
	Log::Out(LogLevel::Error, (tag), "%s in file(%s) line(%d) %s", (message), __FILE__, __LINE__, GetLastWin32Error().c_str());
}

#ifndef K3DPLATFORM_OS_WIN
#define __debugbreak __builtin_trap
#endif

#define K3D_STRINGIFY(x) #x
#define K3D_STRINGIFY_BUILTIN(x) K3D_STRINGIFY(x)
#define K3D_ASSERT(isFalse, ...) \
	if (!(bool)(isFalse)) { \
		::k3d::Log::Out(LogLevel::Error,"Assert","\nAssertion failed in " K3D_STRINGIFY_BUILTIN( __FILE__ ) " @ " K3D_STRINGIFY_BUILTIN( __LINE__ ) "\n"); \
		::k3d::Log::Out(LogLevel::Error,"Assert","\'" #isFalse "\' is false"); \
		::k3d::Log::Out(LogLevel::Error,"Assert", "args is" ##__VA_ARGS__); \
		__debugbreak(); \
    }

#endif
