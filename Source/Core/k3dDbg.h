#ifndef __k3dDbg_h__
#define __k3dDbg_h__
#pragma once
#include <Config/Prerequisities.h>

class k3dDebug
{
public:
  k3dDebug();
  k3dDebug & operator << (const char *);
};

extern void kDebug( const char *fmt, ... );

#endif
