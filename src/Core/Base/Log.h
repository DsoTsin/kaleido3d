#pragma once
#ifndef __LogUtil_h__
#define __LogUtil_h__

namespace k3d
{
	enum class ELogLevel
	{
		Default,
		Debug,
		Info, /** [Message] */
		Warn, /** [Warning] */
		Error,/** [Error]*/
		Fatal, /** [Fatal]*/
		Profile
	};

    class ILogger
    {
    public:
        virtual ~ILogger() {}
        virtual bool IsCancelled() const { return false; }
        virtual void Log(ELogLevel const &, const char * tag, const char *) = 0;
	};

	enum class ELoggerType : U32
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


	extern K3D_CORE_API void Log(ELogLevel const & Lv, const char* tag, const char *fmt, ...);
}

#if !K3DPLATFORM_OS_WINDOWS
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

#endif
