#pragma once
#ifndef __Config_H__
#define __Config_H__
//--------------------------- Project Global Config --------------------------//

/** OS Definitions **/
#ifdef _WIN32
	#define K3DPLATFORM_OS_WIN 1
#elif defined(__linux) || defined(linux)
	#define K3DPLATFORM_OS_LINUX 1
#elif defined(MAC_OSX)
	#define K3DPLATFORM_OS_MAC 1
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
  #endif
  #define KFORCE_INLINE __forceinline
#endif
#endif

#if __GNUC__
  #ifdef __clang__
	#define K3DCOMPILER_CLANG 1
  #else
	#define K3DCOMPILER_GCC 1
  #endif
  #define KFORCE_INLINE inline
#endif
/*** End Compiler Detection ***/



#if K3DPLATFORM_OS_WIN
    #define KALIGN( X ) __declspec( align(X) )
#else
    #define KALIGN( X ) __attribute__( (aligned(X)) )
#endif



/**
 * Class Macros
**/
#define K3DDISCOPY(CLASSNAME)\
  CLASSNAME & operator = (const CLASSNAME &);\
  CLASSNAME( CLASSNAME const & );

#define K3D_UNUSED(x) (void)x

#if defined(K3DPLATFORM_OS_WIN)
#define K3D_APPLICATION_NAME L"kaleido3d app"
#endif

#endif
