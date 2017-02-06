#ifndef __IIODevice_H__
#define __IIODevice_H__
#pragma once

#include "../Math/kTypeTrait.hpp"
#include "../Config/Config.h"
#include "../Config/PlatformTypes.h"
#include <assert.h>

enum IOFlag
{
  IORead=1,
  IOWrite
};

struct K3D_API IIODevice
{
  ~IIODevice() {}
  virtual bool      Open(const k3d::kchar* fileName, IOFlag mode) = 0;
  virtual bool      IsEOF() = 0;
  virtual size_t    Read( char*, size_t ) = 0;
  virtual size_t    Write(const void*, size_t ) = 0;
  virtual bool      Seek( size_t offset ) = 0;
  virtual bool      Skip( size_t offset ) = 0;
  virtual void      Flush() = 0;
  virtual void      Close() = 0;
};

KTYPE_META_TEMPLATE( IIODevice );

template <class IsIODevice>
IIODevice * GetIODevice()
{
  assert(is_IIODevice<IsIODevice>::_YES);
  return IsIODevice::CreateIOInterface();
}

#endif
