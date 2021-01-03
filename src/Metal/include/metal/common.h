#pragma once

#define _MTL_ENUM(x, name) enum name
#define _MTL_OPTIONS(x, name) enum name

#ifdef _MSC_VER
#ifndef WITH_STATIC_LIBRARY
#ifdef BUILD_SHARED_LIB
#define METAL_API __declspec(dllexport)
#else
#define METAL_API __declspec(dllimport)
#endif
#else
#define METAL_API 
#endif
#else
#define METAL_API __attribute((visibility("default")))
#endif

#include <stdint.h>