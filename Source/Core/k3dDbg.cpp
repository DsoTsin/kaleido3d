#include "k3dDbg.h"
#include <Config/OSHeaders.h>
#include <cstdarg>

k3dDebug::k3dDebug()
{}

k3dDebug & k3dDebug::operator<<(const char *str)
{
#ifdef K3DPLATFORM_OS_WIN
  OutputDebugStringA( str );
#else
  fputs( str, stderr );
  fflush( stderr );
#endif
  return *this;
}


void kDebug( const char *fmt, ... )
{
  va_list va;
  char dbgStr[ 1024 ];
  va_start( va, fmt );
  ::vsprintf( dbgStr, fmt, va ); //!to fix: printf %d first argument error
  va_end( va );
  k3dDebug() << dbgStr;
}
