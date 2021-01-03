#ifndef __metal_compiler_h__
#define __metal_compiler_h__
#pragma once

#ifdef _MSC_VER
#ifndef BUILD_LIB
#define METALCOMPILER_API __declspec(dllimport)
#else
#define METALCOMPILER_API __declspec(dllexport)
#endif
#else
#define METALCOMPILER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef void *mtl_process_t;
/// <summary>
///	Invoke metal toolchain
/// https://developer.apple.com/documentation/metal/shader_libraries/building_a_library_with_metal_s_command-line_tools
/// metal -c MyLibrary.metal -o MyLibrary.air
/// metallib MyLibrary.air -o MyLibrary.metallib
/// </summary>
/// <param name="is_ios">0 or 1</param>
/// <param name="program">metal/metallib</param>
/// <param name="args"></param>
/// <param name="process"></param>
/// <returns></returns>
METALCOMPILER_API int metal_run_process(int is_ios, const char* program, const char *args, mtl_process_t *process);
METALCOMPILER_API int metal_close_process(mtl_process_t process);

#ifdef __cplusplus
}
#endif

#endif