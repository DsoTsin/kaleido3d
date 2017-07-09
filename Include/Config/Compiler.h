#pragma once
#include "Config.h"
#include "PlatformTypes.h"

#if defined(K3D_CPP_REFLECTOR)
#define k3dAnnotate(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define k3dAnnotate(...)  
#endif

#if K3DCOMPILER_MSVC
#pragma warning(disable:4100)
#pragma warning(disable:4838) // close warning from DirectXMath
#pragma warning(disable:4127)
#pragma warning(disable:4251)
#define STD_CALL __stdcall
#endif

#if K3DCOMPILER_CLANG
#define STD_CALL 
#endif

#if K3DCOMPILER_MSVC
#define __K3D_FUNC__ __FUNCTION__
#else 
#define __K3D_FUNC__ __func__
#endif

/// Define intrinsics
namespace __k3d_intrinsics__
{
	inline int32 AtomicIncrement(int32* i32)
	{
#if defined(K3DCOMPILER_CLANG) || (defined(K3DCOMPILER_GCC) && K3DCOMPILER_VERSION > 4003)
		return __sync_add_and_fetch(i32, 1);
#elif defined(K3DCOMPILER_MSVC)
		static_assert(sizeof(long) == sizeof(int32), "unexpected size");
		return _InterlockedIncrement((volatile long*)i32);
#elif defined(K3DCOMPILER_GCC)
		int32 result;
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

	inline int32 AtomicDecrement(int32* i32)
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

	inline bool AtomicCAS(int32* i32, int32 newValue, int32 condition)
	{
#if defined(K3DCOMPILER_CLANG) || (defined(K3DCOMPILER_GCC) && K3DCOMPILER_VERSION > 4003)
		return __sync_bool_compare_and_swap(i32, condition, newValue);
#elif defined(K3DCOMPILER_MSVC)
		return ((int32)_InterlockedCompareExchange((volatile long*)i32, (long)newValue, (long)condition) == condition);
#elif defined(K3DCOMPILER_GCC)
		int32 result;
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
}