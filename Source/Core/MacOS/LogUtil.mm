#include "Kaleido3D.h"
#include "LogUtil.h"
#include "ModuleCore.h"
#include <cstdarg>
#include "File.h"
#include <mutex>

#include "../LogUtil.inl"
#include "../../../Data/style.css.h"

#import <Foundation/Foundation.h>

static inline void DebugOut(const char * dbgBuffer) {
    NSLog(@ "％s", dbgBuffer);
}

namespace k3d {
    
    
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
        ::vsprintf(dbgStr, fmt, va); //!to fix: printf %d first argument error
        va_end(va);
        ::sprintf(dbgBuffer, "[%s]::%s\n", tag, dbgStr);
        DebugOut(dbgBuffer);
    }
    
    void Log::Out(const char * tag, std::string const & log)
    {
        std::string outLog("[");
        outLog += tag;
        outLog += "]";
        outLog += log;
        DebugOut(outLog.c_str());
    }
    
    void Log::Out(LogLevel && lev, const char * tag, const char * fmt, ...)
    {
        va_list va;
        static char dbgStr[2048] = { 0 };
        static char dbgBuffer[2048] = { 0 };
        va_start(va, fmt);
        ::vsprintf(dbgStr, fmt, va); //!to fix: printf %d first argument error
        va_end(va);
        ::sprintf(dbgBuffer, "[%s]::%s\n", tag, dbgStr);
        Log::Get().LogOutPut(lev, dbgBuffer);
    }
    
    void Log::LogOutPut(LogLevel lev, const char * buffer) {
        std::lock_guard<std::mutex> lock(s_LogLock);
        std::string logLineHead, logLineTail;
        DebugOut(buffer);
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
    }
    
    void Log::Destroy()
    {
        if (s_LogFile) {
            OutputStr2IODevice(s_LogFile, tail);
            s_LogFile->Flush();
            s_LogFile->Close();
        }
    }
    
    Log::Log() : s_LogFile(nullptr) {
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
    }
    
}