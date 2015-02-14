#include "Kaleido3D.h"
#include "LogUtil.h"
#include <Config/OSHeaders.h>
#include <cstdarg>
#include <cstdio>
#include "File.h"
#include <mutex>

namespace k3d {
    
    namespace Global{
        static Debug::OutPutCallBack output = nullptr;
    }
    
	Debug::Debug()
	{}

    void Debug::SetDebugOutFunction(Debug::OutPutCallBack callBack)
    {
        Global::output = callBack;
    }
    
	Debug & Debug::operator<<(const char *str)
	{
#ifdef K3DPLATFORM_OS_WIN
		OutputDebugStringA(str);
#elif K3DPLATFORM_OS_LINUX
		fputs(str, stderr);
		fflush(stderr);
#else
        if(nullptr!=Global::output)
        {
            Global::output(str);
        }
#endif
		return *this;
	}

	Debug & Debug::operator<<(std::string const & str)
	{
#ifdef K3DPLATFORM_OS_WIN
		OutputDebugStringA(str.c_str());
#elif K3DPLATFORM_OS_LINUX
		fputs(str, stderr);
        fflush(stderr);
#else
        if(nullptr!=Global::output)
        {
            Global::output(str.c_str());
        }
#endif
		return *this;
	}

	void Debug::Out(const char * tag, const char * fmt, ...)
	{
		va_list va;
		static char dbgStr[2048] = { 0 };
		static char dbgBuffer[2048] = { 0 };
		va_start(va, fmt);
		::vsprintf(dbgStr, fmt, va); //!to fix: printf %d first argument error
		va_end(va);
		::sprintf(dbgBuffer, "[%s]::%s\n", tag, dbgStr);
#if   K3DPLATFORM_OS_WIN
		OutputDebugStringA(dbgBuffer);
#elif K3DPLATFORM_OS_LINUX
		fputs(dbgBuffer, stderr);
        fflush(stderr);
#else
        if(nullptr!=Global::output)
        {
            Global::output(dbgBuffer);
        }
#endif
	}

	void Debug::Out(const char * tag, std::string const & log)
	{
		std::string outLog("[");
		outLog += tag;
		outLog += "]";
		outLog += log;
		Debug() << outLog ;
	}

	IIODevice *g_LogFile = nullptr;
	std::mutex g_LogLock;

	static void OutputStr2IODevice(IIODevice* device, const char *str)
	{
		assert(device != NULL);
		while (char c = *str++)
			device->Write(&c, 1);
		//Need Flush Data EveryTime?
		//device->Flush();
	}

	static const char* GetLocalTime() {
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		static char time_info[128] = { 0 };
		::sprintf(time_info, "%02d:%02d:%02d ", tm->tm_hour, tm->tm_min, tm->tm_sec);
		return time_info;
	}

	static std::string Txt2Html(const char *str) {
		std::string ret(GetLocalTime());
		const char *s = str;
		while (*s) {
			if (*s == '\t') {
				ret += "&nbsp; &nbsp; ";
			}
			else if (*s == '<') {
				ret += "&lt;";
			}
			else if (*s == '>') {
				ret += "&gt;";
			}
			else if (*s == '\n') {
				if (*(s + 1)) ret += "<br/>" + std::string(GetLocalTime());
			}
			else {
				ret += *s;
			}
			s++;
		}
		return ret;
	}

	bool Log::InitLogFile(const char *name)
	{
		std::string path;
		const char * env = getenv("Kaleido3D_Dir");
		path = (env == nullptr) ? "./" : env;
		path += "/" + std::string(name);
		if (g_LogFile == NULL)
			g_LogFile = GetIODevice<File>();
		if (!g_LogFile->Open(path.c_str(), IOWrite)) return false;
		OutputStr2IODevice(g_LogFile, "<!DOCTYPE html>\n<html>\n");
		OutputStr2IODevice(g_LogFile, "<head>\n\t<meta charset=\"utf-8\"/><title>kaleido3d log</title>");
		OutputStr2IODevice(g_LogFile, "\n\t<link rel=\"stylesheet\" href=\"./k3dLog/style.css\">\n</head>");
		OutputStr2IODevice(g_LogFile, "\n<body>\n\t<nav id=\"nav\" class=\"nav\">\n\t\t<div class=\"alignleft\">\n\t\t\t");
		OutputStr2IODevice(g_LogFile, "<a href=\"./\" class=\"logo\"><img src=\"./k3dLog/image/logo.png\" alt=\"TsinStudio\"></a>\n\t\t");
		OutputStr2IODevice(g_LogFile, "</div>\n\t\t<div class=\"alignright\"><ul class =\"menu\">\n\t\t\t<li><a>Home</a></li></ul>\n\t\t</div>");
		OutputStr2IODevice(g_LogFile, "\n\t</nav>\n\t<section id=\"homepage\" class=\"fullscreen\">\n\t\t<div class=\"container claim\">\n\t\t\t<div class=\"span12\">");
		OutputStr2IODevice(g_LogFile, "\n\t\t\t\t<h1 class=\"aligncenter\">Kaleido3d Log</h1>\n\t\t\t</div>\n\t\t</div>\n\t\t<div class=\"container tmpMargin textOstec\">\n\t\t\t<div class =\"span12\">");
		g_LogFile->Flush();
		return true;
	}

	void Log::CloseLog()
	{
		if (g_LogFile)
		{
			//    std::unique_lock<std::mutex> lock( g_LogLock );
			OutputStr2IODevice(g_LogFile, "\n\t\t\t</div>\n\t\t</div>\n\t</section>\n\t<footer>\n\t<style>{margin:0px; padding:0px; }</style>\n\t</footer>\n</body>\n</html>");
			g_LogFile->Flush();
			g_LogFile->Close();
		}
	}

	void Log::Message(const char *fmt, ...)
	{
		std::unique_lock<std::mutex> lock(g_LogLock);
		va_list va;
		va_start(va, fmt);
		static char data[1024] = { 0 };
		::vsprintf(data, fmt, va);
		va_end(va);
		std::string line = "<h2>" + Txt2Html(data) + "</h2>\n\t\t\t\t";
		OutputStr2IODevice(g_LogFile, line.c_str());
	}

	void Log::Warning(const char *fmt, ...)
	{
		std::unique_lock<std::mutex> lock(g_LogLock);
		va_list va;
		va_start(va, fmt);
		static char data[1024] = { 0 };
		::vsprintf(data, fmt, va);
		va_end(va);

		std::string line = "<h5>" + Txt2Html(data) + "</h5>\n\t\t\t\t";
		OutputStr2IODevice(g_LogFile, line.c_str());
	}

	void Log::Error(const char *fmt, ...)
	{
		std::unique_lock<std::mutex> lock(g_LogLock);
		va_list va;
		va_start(va, fmt);
		static char data[1024] = { 0 };
		::vsprintf(data, fmt, va);
		va_end(va);

		std::string line = "<h3>" + Txt2Html(data) + "</h3>\n\t\t\t\t";
		OutputStr2IODevice(g_LogFile, line.c_str());
	}

	void Log::Fatal(const char *fmt, ...)
	{
		std::unique_lock<std::mutex> lock(g_LogLock);
		va_list va;
		va_start(va, fmt);
		static char data[1024] = { 0 };
		::vsprintf(data, fmt, va);
		va_end(va);

		std::string line = "<h4>" + Txt2Html(data) + "</h4>\n\t\t\t\t";
		OutputStr2IODevice(g_LogFile, line.c_str());
	}

///=======================================================================================
	
	LogUtil::LogCall LogUtil::s_UsrDefCall = nullptr;

	void LogUtil::Out(LogLevel && lev, const char * tag, const char *fmt, ...)
	{
		va_list va;
		static char logBuffer[2048] = { 0 };
		va_start(va, fmt);
		::vsprintf(logBuffer, fmt, va); //!to fix: printf %d first argument error
		va_end(va);

		if (s_UsrDefCall != nullptr)
		{
			s_UsrDefCall(lev, tag, logBuffer);
			return;
		}

		switch (lev)
		{
		case LogLevel::Debug:
			Debug::Out(tag, logBuffer);
			break;
		case LogLevel::Info:
			break;
		case LogLevel::Warn:
			break;
		case LogLevel::Error:
			break;
		case LogLevel::Fatal:
			break;
		default:
			break;
		}
	}

}