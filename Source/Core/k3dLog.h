#ifndef __k3dLog_h__
#define __k3dLog_h__
#pragma once
#include <Config/Prerequisities.h>

class k3dLog
{
  k3dLog();
public:
  static bool   InitLogFile( const char *name );
  static void   CloseLog();

  static void   Message( const char *format, ... );
  static void   Warning( const char *format, ... );
  static void   Error( const char *format, ... );
  static void   Fatal( const char *format, ... );
};

#define LOG_WARN(expression, message) \
    if(!(expression)) k3dLog::Warning(message);

#define LOG_MESSAGE(message) \
    k3dLog::Message(message);

#endif
