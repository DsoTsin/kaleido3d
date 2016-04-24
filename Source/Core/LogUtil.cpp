#include "Kaleido3D.h"
#include "LogUtil.h"
#include "ModuleCore.h"
#include <cstdarg>
#include "File.h"
#include <mutex>
#include <Config/OSHeaders.h>
#include "LogUtil.inl"
#include "../../Data/style.css.h"

namespace k3d {

    
    static inline void DebugOut(const char * dbgBuffer) {
#if   K3DPLATFORM_OS_WIN
        OutputDebugStringA(dbgBuffer);
#elif K3DPLATFORM_OS_LINUX && !K3DPLATFORM_OS_ANDROID
        fputs(dbgBuffer, stderr);
        fflush(stderr);
#elif K3DPLATFORM_OS_ANDROID
        __android_log_print(ANDROID_LOG_DEFAULT, "k3d", dbgBuffer);
#else
        //NSLog(@ "％s", dbgBuffer);
#endif
    }

#if K3DPLATFORM_OS_ANDROID

	android_LogPriority prios [] = {
			ANDROID_LOG_DEBUG,
			ANDROID_LOG_INFO,
			ANDROID_LOG_WARN,
			ANDROID_LOG_ERROR,
			ANDROID_LOG_FATAL
	};

#endif

    void Log::SetDebugOutFunction(Log::OutPutCallBack callBack)
    {
    }
    
	Log & Log::operator<<(const char *str)
	{
		DebugOut(str);
		return *this;
	}

	Log & Log::operator<<(std::string const & str)
	{
		DebugOut(str.c_str());
		return *this;
	}

	void Log::Out(const char * tag, const char * fmt, ...)
	{
		va_list va;
		static char dbgStr[2048] = { 0 };
		static char dbgBuffer[2048] = { 0 };
		va_start(va, fmt);
#if K3DPLATFORM_OS_ANDROID
		::__android_log_print(ANDROID_LOG_VERBOSE, tag, fmt, va);
#else
		::vsprintf(dbgStr, fmt, va); //!to fix: printf %d first argument error
#endif
		va_end(va);
#if !K3DPLATFORM_OS_ANDROID
		::sprintf(dbgBuffer, "[%s]::%s\n", tag, dbgStr);
		DebugOut(dbgBuffer);
#endif
	}

	void Log::Out(const char * tag, std::string const & log)
	{
		std::string outLog("[");
		outLog += tag;
		outLog += "]";
		outLog += log;
		DebugOut(outLog.c_str());
	}

	void Log::Out(LogLevel const& lev, const char * tag, const char * fmt, ...)
	{
		va_list va;
		static char dbgStr[2048] = { 0 };
		static char dbgBuffer[2048] = { 0 };
		va_start(va, fmt);
#if K3DPLATFORM_OS_ANDROID
		::vsprintf(dbgStr, fmt, va);
#else
		::vsprintf(dbgStr, fmt, va);
#endif
		va_end(va);
#if !K3DPLATFORM_OS_ANDROID
		::sprintf(dbgBuffer, "[%s]::%s\n", tag, dbgStr);
		Log::Get().LogOutPut(lev, dbgBuffer);
#else
		::__android_log_print(prios[(uint32)lev], tag, dbgStr);
#endif
	}

	void Log::LogOutPut(LogLevel lev, const char * buffer) {
		DebugOut(buffer);
#ifndef K3DPLATFORM_OS_ANDROID
		::Concurrency::Mutex::AutoLock lock(&s_LogLock);
		std::string logLineHead, logLineTail;
		switch (lev)
		{
		case LogLevel::Debug:
			break;
		case LogLevel::Info:
			logLineHead = "<h2>";
			logLineTail = "</h2>\n\t\t\t\t";
			break;
		case LogLevel::Warn:
			logLineHead = "<h3>";
			logLineTail = "</h3>\n\t\t\t\t";
			break;
		case LogLevel::Error:
			logLineHead = "<h4>";
			logLineTail = "</h4>\n\t\t\t\t";
			break;
		case LogLevel::Fatal:
			logLineHead = "<h5>";
			logLineTail = "</h5>\n\t\t\t\t";
			break;
		default:
			break;
		}
		std::string line = logLineHead + Txt2Html(buffer) + logLineTail;
		OutputStr2IODevice(s_LogFile, line.c_str());
#endif
	}

	void Log::Destroy()
	{
#ifndef K3DPLATFORM_OS_ANDROID
		if (s_LogFile) {
			OutputStr2IODevice(s_LogFile, tail);
			s_LogFile->Flush();
			s_LogFile->Close();
		}
#endif
	}

	Log::Log() : s_LogFile(nullptr) {
#ifndef K3DPLATFORM_OS_ANDROID
		kString path = Core::GetExecutablePath() + KT("/log.html");
		if (s_LogFile == nullptr)
			s_LogFile = GetIODevice<File>();
		if (!s_LogFile->Open(path.c_str(), IOWrite))
			return;
		OutputStr2IODevice(s_LogFile, header);
		s_LogFile->Flush();
		kString stylePath = Core::GetExecutablePath() + KT("/style.css");
		File styleFile;
		styleFile.Open(stylePath.c_str(), IOWrite);
		styleFile.Write(style_css, sizeof(style_css));
		styleFile.Close();
#endif
	}

}