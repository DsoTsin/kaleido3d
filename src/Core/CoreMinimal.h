#pragma once

#ifndef __k3d_CoreMinimal__
#define __k3d_CoreMinimal__

#define K3DPLATFORM_OS_WINDOWS (K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_WINUWP)
#define K3DPLATFORM_OS_UNIX (K3DPLATFORM_OS_LINUX || K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_IOS || K3DPLATFORM_OS_ANDROID)
#define K3DPLATFORM_OS_APPLE (K3DPLATFORM_OS_MAC || K3DPLATFORM_OS_IOS)

#include <stdint.h>

/** Compiler Definiotions **/
#if K3DPLATFORM_OS_WINDOWS
#ifdef _MSC_VER
  #if (_MSC_VER < 1800)
	  #pragma message ("Not Support C++ Compiler Version Under MS Visual C++ 110! For This Project Need C++11 Support")
    #error "Fatal Error : Your Compiler doesn't Support c++11 !"
  #else
    #define K3DCOMPILER_MSVC 1
    #define _CRT_SECURE_NO_WARNINGS 1
	extern "C" long  _InterlockedIncrement(long volatile* Addend);
	#pragma intrinsic (_InterlockedIncrement)
	extern "C" long _InterlockedDecrement(long volatile* Addend);
	#pragma intrinsic (_InterlockedDecrement)
	extern "C" long _InterlockedCompareExchange(long volatile* Dest, long Exchange, long Comp);
	#pragma intrinsic (_InterlockedCompareExchange)

    // 64 bits
    extern "C" int64_t _InterlockedCompareExchange64(int64_t volatile* Dest, int64_t Exchange, int64_t Comp);
    #pragma intrinsic (_InterlockedCompareExchange64)
    extern "C" void * _InterlockedCompareExchangePointer(
        void * volatile * Destination,
        void * Exchange,
        void * Comparand
    );
    #pragma intrinsic (_InterlockedCompareExchangePointer)
  #endif
	#define KRESTRICT __restrict
	#define KFORCE_INLINE __forceinline
	#define FARMHASH_NO_BUILTIN_EXPECT
    #define KPACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif
#endif

#if __GNUC__
  #ifdef __clang__
	#define K3DCOMPILER_CLANG 1
  #else
	#define K3DCOMPILER_GCC 1
  #endif
  #define KFORCE_INLINE inline
  #define KRESTRICT __restrict__
  #define K3DCOMPILER_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
  #define KPACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif
/*** End Compiler Detection ***/

#define K3D_NOEXCEPT noexcept

#if K3DPLATFORM_OS_WINDOWS
    #define KALIGN( X ) __declspec( align(X) )
#else
    #define KALIGN( X ) __attribute__( (aligned(X)) )
	#define STD_CALL     
#endif

#define K3D_UNUSED(x) (void)x

#if K3DPLATFORM_OS_WINDOWS
	#if defined(LIB_BUILD)
		#if defined(BUILD_SHARED_LIB)
			#define K3D_CORE_API __declspec(dllexport)
		#else
			#define K3D_CORE_API __declspec(dllimport)
		#endif
	#else
		#define K3D_CORE_API     
	#endif
#else
    #define K3D_CORE_API __attribute__((visibility("default"))) 
#endif

#if defined(K3D_CPP_REFLECTOR)
#define KCLASS(...) __attribute__((annotate(#__VA_ARGS__)))
#define KSTRUCT(...) __attribute__((annotate(#__VA_ARGS__)))
#define KFUNCTION(...) __attribute__((annotate(#__VA_ARGS__)))
#define KPROPERTY(...) __attribute__((annotate(#__VA_ARGS__)))
#define KENUM(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define KCLASS(...)  
#define KSTRUCT(...)  
#define KFUNCTION(...)  
#define KPROPERTY(...)  
#define KENUM(...)  
#endif

#if K3DCOMPILER_MSVC
#pragma warning(disable:4100)
#pragma warning(disable:4838) // close warning from DirectXMath
#pragma warning(disable:4127)
#pragma warning(disable:4251)
#define STD_CALL __stdcall
#define K3D_DEPRECATED(text)  __declspec(deprecated(text))
#else
#define K3D_DEPRECATED(text) __attribute__((__deprecated__(text)))
#endif

#if K3DCOMPILER_CLANG
#define STD_CALL 
#endif

#if K3DCOMPILER_MSVC
#define __K3D_FUNC__ __FUNCTION__
#else 
#define __K3D_FUNC__ __func__
#endif

#include <stdlib.h>
#include <assert.h>

/// Define intrinsics
namespace __intrinsics__
{
	inline int32_t AtomicIncrement(int32_t* i32)
	{
#if defined(K3DCOMPILER_CLANG) || (defined(K3DCOMPILER_GCC) && K3DCOMPILER_VERSION > 4003)
		return __sync_add_and_fetch(i32, 1);
#elif defined(K3DCOMPILER_MSVC)
		static_assert(sizeof(long) == sizeof(int32_t), "unexpected size");
		return _InterlockedIncrement((volatile long*)i32);
#elif defined(K3DCOMPILER_GCC)
		int32_t result;
		__asm__ __volatile__("lock; xaddl %0, %1"
			: "=r" (result), "=m" (*i32)
			: "0" (1), "m" (*i32)
			: "memory"
		);
		return result + 1;
#else
		return ++*i32;
#endif
	}

	inline int32_t AtomicDecrement(int32_t* i32)
	{
#if defined(K3DCOMPILER_CLANG) || (defined(K3DCOMPILER_GCC) && K3DCOMPILER_VERSION > 4003)
		return __sync_add_and_fetch(i32, -1);
#elif defined(K3DCOMPILER_MSVC)
		return _InterlockedDecrement((volatile long*)i32);
#elif defined(K3DCOMPILER_GCC)
		int32 result;
		__asm__ __volatile__("lock; xaddl %0, %1"
			: "=r" (result), "=m" (*i32)
			: "0" (-1), "m" (*i32)
			: "memory"
		);
		return result - 1;
#else
		return --*i32;
#endif
	}

	inline bool AtomicCAS(int32_t* i32, int32_t newValue, int32_t condition)
	{
#if defined(K3DCOMPILER_CLANG) || (defined(K3DCOMPILER_GCC) && K3DCOMPILER_VERSION > 4003)
		return __sync_bool_compare_and_swap(i32, condition, newValue);
#elif defined(K3DCOMPILER_MSVC)
		return ((long)_InterlockedCompareExchange((volatile long*)i32, (long)newValue, (long)condition) == condition);
#elif defined(K3DCOMPILER_GCC)
		int32_t result;
		__asm__ __volatile__(
			"lock; cmpxchgl %3, (%1) \n"                    // Test *i32 against EAX, if same, then *i32 = newValue 
			: "=a" (result), "=r" (i32)                     // outputs
			: "a" (condition), "r" (newValue), "1" (i32)    // inputs
			: "memory"                                      // clobbered
		);
		return result == condition;
#else
		if (*i32 == condition)
		{
			*i32 = newValue;
			return true;
		}
		return false;
#endif
	}

    inline bool AtomicCAS64(int64_t* i64, int64_t newValue, int64_t comparand)
    {
#if defined(K3DCOMPILER_MSVC)
        return _InterlockedCompareExchange64(i64, newValue, comparand) == comparand;
#else

#endif
    }

    /*
     * @param Destination
     * @param NewValue to assign
     * @param OldValue to compare
     */
    inline bool AtomicCASPointer(
        void ** Destination,
        void * NewValue,
        void * OldValue)
    {
#if defined(K3DCOMPILER_MSVC)
        return _InterlockedCompareExchangePointer((void*volatile*)Destination, NewValue, OldValue) == OldValue;
#else

#endif
    }
}

// The follow include order cannot be modified, otherwise could cause massive compiler errors !!

#include "Base/Types.h"
#include "Base/Simd.h"
#include "Base/IO.h"
#include "Base/Memory.h"

#include "KTL/TypeTrait.h"

#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Quaterion.h"
#include "Math/Geometry.h"

#include "KTL/Allocator.h"
#include "KTL/Atomic.h"
#include "KTL/Algorithm.h"
#include "KTL/DynArray.h"
#include "KTL/SparseArray.h"
#include "KTL/Circular.h"
#include "KTL/String.h"
#include "KTL/SharedPtr.h"

#include "Base/Module.h"
#include "Base/Log.h"
#include "Base/Encoder.h"
//#include "Base/Regex.h"
#include "Base/Profiler/Profiler.h"

#include "XPlatform/App.h"
#include "XPlatform/Os.h"
#include "XPlatform/Window.h"

#include "KTL/LockFreeQueue.h"

#include "Net/Net.h"


#endif
