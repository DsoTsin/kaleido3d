#pragma once
#ifndef __ngfx_atomic_h__
#define __ngfx_atomic_h__

#include <stdint.h>

namespace ngfx
{
#if defined(_MSC_VER)
extern "C" long  _InterlockedIncrement(long volatile* Addend);
#pragma intrinsic (_InterlockedIncrement)
extern "C" long _InterlockedDecrement(long volatile* Addend);
#pragma intrinsic (_InterlockedDecrement)
extern "C" long _InterlockedCompareExchange(long volatile* Dest, long Exchange, long Comp);
#pragma intrinsic (_InterlockedCompareExchange)

// 64 bits
extern "C" int64_t _InterlockedCompareExchange64(int64_t volatile* Dest, int64_t Exchange, int64_t Comp);
#pragma intrinsic (_InterlockedCompareExchange64)
extern "C" void * _InterlockedCompareExchangePointer(void * volatile * Destination, void * Exchange, void * Comparand);
#pragma intrinsic (_InterlockedCompareExchangePointer)
#endif

	inline int32_t atomic_increment(int32_t* i32)
	{
#if defined(__clang__) || defined(__GNUC__)
		return __sync_add_and_fetch(i32, 1);
#elif defined(_MSC_VER)
		static_assert(sizeof(long) == sizeof(int32_t), "unexpected size");
		return _InterlockedIncrement((volatile long*)i32);
#elif defined(__GNUC__)
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
	inline int32_t atomic_decrement(int32_t* i32)
	{
#if defined(__clang__) || defined(__GNUC__)
		return __sync_add_and_fetch(i32, -1);
#elif defined(_MSC_VER)
		return _InterlockedDecrement((volatile long*)i32);
#elif defined(__GNUC__)
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
	inline bool atomic_cas(int32_t* i32, int32_t newValue, int32_t condition)
	{
#if defined(__clang__) || defined(__GNUC__)
		return __sync_bool_compare_and_swap(i32, condition, newValue);
#elif defined(_MSC_VER)
		return ((long)_InterlockedCompareExchange((volatile long*)i32, (long)newValue, (long)condition) == condition);
#elif defined(__GNUC__)
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
	typedef uint8_t uint8;
	typedef uint32_t uint32;
	typedef uint64_t uint64;
}

#endif