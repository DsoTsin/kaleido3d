#pragma once
#include "Config.h"

#if K3DCOMPILER_MSVC
#pragma warning(disable:4100)
#pragma warning(disable:4838) // close warning from DirectXMath
#define STD_CALL __stdcall
#endif

#if K3DCOMPILER_CLANG
#define STD_CALL 
#endif