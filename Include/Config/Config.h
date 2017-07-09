#pragma once
#ifndef __Config_H__
#define __Config_H__
//--------------------------- Project Global Config --------------------------//

/** OS Definitions **/
#ifdef _WIN32
	#define K3DPLATFORM_OS_WIN 1
#elif defined(__linux) || defined(linux)
	#define K3DPLATFORM_OS_LINUX 1
#elif defined(K3DPLATFORM_OS_MAC)
	#define K3DPLATFORM_OS_MAC 1
#elif defined(K3DPLATFORM_OS_IOS)
    #define K3DPLATFORM_OS_IOS 1
#else
	#error "Not Support This Platform OS!!"
#endif

/** Compiler Definiotions **/
#if K3DPLATFORM_OS_WIN
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
  #endif
	#define KRESTRICT __restrict
	#define KFORCE_INLINE __forceinline
	#define FARMHASH_NO_BUILTIN_EXPECT
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
#endif
/*** End Compiler Detection ***/

#define K3D_NOEXCEPT noexcept

#if K3DPLATFORM_OS_WIN
    #define KALIGN( X ) __declspec( align(X) )
#else
    #define KALIGN( X ) __attribute__( (aligned(X)) )
	#define STD_CALL     
#endif



/**
 * Class Macros
**/
#define K3D_DISCOPY(CLASSNAME)\
  CLASSNAME & operator = (const CLASSNAME &);\
  CLASSNAME( CLASSNAME const & );\
  CLASSNAME( CLASSNAME const && );

#define K3D_UNUSED(x) (void)x

#if defined(K3DPLATFORM_OS_WIN)
#define K3D_APPLICATION_NAME L"kaleido3d app"
//#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
	#if defined(LIB_BUILD)
		#if defined(BUILD_SHARED_LIB)
			#define K3D_API __declspec(dllexport)
		#else
			#define K3D_API __declspec(dllimport)
		#endif
	#else
		#define K3D_API     
	#endif
#else
    #define K3D_API __attribute__((visibility("default"))) 
#endif

#define K3D_COMMON_NS namespace k3d

#define K3D_DEPRECATED(...)  

#endif
