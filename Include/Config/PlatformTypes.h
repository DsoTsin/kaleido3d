#pragma once
#ifndef __Types_H__
#define __Types_H__

#if K3DPLATFORM_OS_IOS
    #include "stdint.h"
#else
    #include <cstdint>
#endif
//#include <float.h>

typedef int8_t    int8;
typedef int16_t   int16;
typedef int32_t   int32;

typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef int64_t   int64;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
typedef unsigned char kByte;

namespace k3d {
#if K3DPLATFORM_OS_WIN
    typedef wchar_t kchar;
    #define KT(quote) L##quote
#else
    #define KT(quote) quote
    typedef char	kchar;
#endif
}

#endif
