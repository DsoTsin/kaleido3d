#pragma once

#ifndef __k3d_Types_h__
#define __k3d_Types_h__

#if K3D_USE_SSE
#include <emmintrin.h>
#elif K3D_USE_NEON
#include <arm_neon.h>
#endif

namespace k3d
{
    typedef int8_t    I8;
    typedef int16_t   I16;
    typedef int32_t   I32;

    typedef uint8_t   U8;
    typedef uint16_t  U16;
    typedef int64_t   I64;
    typedef uint32_t  U32;
    typedef uint64_t  U64;

#if K3D_USE_SSE
    typedef __m128  V4F;
    typedef __m128i V4I;
#elif K3D_USE_NEON
    typedef float32x4_t __attribute((aligned(16))) V4F;
    typedef int32x4_t __attribute((aligned(16))) V4I;
#else
    typedef struct
    {
        float f32[4];
    } V4F;
    typedef struct
    {
        union
        {
            U32 u32[4];
            I32 i32[4];
        };
    } V4I;
#endif
    
}

#endif