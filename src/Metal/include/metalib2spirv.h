#ifndef __METALIB2SPIRV__
#define __METALIB2SPIRV__
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef _MSC_VER
#ifndef BUILD_LIB
#define METALIB2SPIRV_API __declspec(dllimport)
#else
#define METALIB2SPIRV_API __declspec(dllexport)
#endif
#else
#define METALIB2SPIRV_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef enum mtl2spv_error {
  MTL2SPV_SUCCESS,
  MTL2SPV_LDR_INVALID_HEADER_SIZE,
  MTL2SPV_LDR_INVALID_HEADER_MAGIC,
  MTL2SPV_LDR_INVALID_LIBRARY_SIZE,
  MTL2SPV_LDR_PARSE_PROGRAM_ERROR,
  MTL2SPV_LDR_INVALID_PROGRAM_HASH_SIZE,
  MTL2SPV_LDR_INVALID_PROGRAM_OFFSET,
  MTL2SPV_LDR_INVALID_TAG,
  MTL2SPV_LDR_INVALID_ENTRY_COUNT,
  MTL2SPV_LDR_FAILED_TO_PARSE_BC,
  MTL2SPV_SPV_WRITE_FAILED,
} mtl2spv_error;

typedef enum mtl2spv_shader_type {
  MTL2SPV_SHADER_VERTEX,
  MTL2SPV_SHADER_FRAGMENT,
  MTL2SPV_SHADER_COMPUTE,
  MTL2SPV_SHADER_UNKNOWN,
} mtl2spv_shader_type;

typedef enum mtl2spv_data_type {
  MTL2SPV_DATA_TYPE_NONE = 0,
  MTL2SPV_DATA_TYPE_STRUCT = 1,
  MTL2SPV_DATA_TYPE_ARRAY = 2,
  MTL2SPV_DATA_TYPE_FLOAT = 3,
  MTL2SPV_DATA_TYPE_FLOAT2 = 4,
  MTL2SPV_DATA_TYPE_FLOAT3 = 5,
  MTL2SPV_DATA_TYPE_FLOAT4 = 6,
  MTL2SPV_DATA_TYPE_FLOAT2x2 = 7,
  MTL2SPV_DATA_TYPE_FLOAT2x3 = 8,
  MTL2SPV_DATA_TYPE_FLOAT2x4 = 9,
  MTL2SPV_DATA_TYPE_FLOAT3x2 = 10,
  MTL2SPV_DATA_TYPE_FLOAT3x3 = 11,
  MTL2SPV_DATA_TYPE_FLOAT3x4 = 12,
  MTL2SPV_DATA_TYPE_FLOAT4x2 = 13,
  MTL2SPV_DATA_TYPE_FLOAT4x3 = 14,
  MTL2SPV_DATA_TYPE_FLOAT4x4 = 15,
  MTL2SPV_DATA_TYPE_HALF = 16,
  MTL2SPV_DATA_TYPE_HALF2 = 17,
  MTL2SPV_DATA_TYPE_HALF3 = 18,
  MTL2SPV_DATA_TYPE_HALF4 = 19,
  MTL2SPV_DATA_TYPE_HALF2x2 = 20,
  MTL2SPV_DATA_TYPE_HALF2x3 = 21,
  MTL2SPV_DATA_TYPE_HALF2x4 = 22,
  MTL2SPV_DATA_TYPE_HALF3x2 = 23,
  MTL2SPV_DATA_TYPE_HALF3x3 = 24,
  MTL2SPV_DATA_TYPE_HALF3x4 = 25,
  MTL2SPV_DATA_TYPE_HALF4x2 = 26,
  MTL2SPV_DATA_TYPE_HALF4x3 = 27,
  MTL2SPV_DATA_TYPE_HALF4x4 = 28,
  MTL2SPV_DATA_TYPE_INT = 29,
  MTL2SPV_DATA_TYPE_INT2 = 30,
  MTL2SPV_DATA_TYPE_INT3 = 31,
  MTL2SPV_DATA_TYPE_INT4 = 32,
  MTL2SPV_DATA_TYPE_UINT = 33,
  MTL2SPV_DATA_TYPE_UINT2 = 34,
  MTL2SPV_DATA_TYPE_UINT3 = 35,
  MTL2SPV_DATA_TYPE_UINT4 = 36,
  MTL2SPV_DATA_TYPE_SHORT = 37,
  MTL2SPV_DATA_TYPE_SHORT2 = 38,
  MTL2SPV_DATA_TYPE_SHORT3 = 39,
  MTL2SPV_DATA_TYPE_SHORT4 = 40,
  MTL2SPV_DATA_TYPE_USHORT = 41,
  MTL2SPV_DATA_TYPE_USHORT2 = 42,
  MTL2SPV_DATA_TYPE_USHORT3 = 43,
  MTL2SPV_DATA_TYPE_USHORT4 = 44,
  MTL2SPV_DATA_TYPE_CHAR = 45,
  MTL2SPV_DATA_TYPE_CHAR2 = 46,
  MTL2SPV_DATA_TYPE_CHAR3 = 47,
  MTL2SPV_DATA_TYPE_CHAR4 = 48,
  MTL2SPV_DATA_TYPE_UCHAR = 49,
  MTL2SPV_DATA_TYPE_UCHAR2 = 50,
  MTL2SPV_DATA_TYPE_UCHAR3 = 51,
  MTL2SPV_DATA_TYPE_UCHAR4 = 52,
  MTL2SPV_DATA_TYPE_BOOL = 53,
  MTL2SPV_DATA_TYPE_BOOL2 = 54,
  MTL2SPV_DATA_TYPE_BOOL3 = 55,
  MTL2SPV_DATA_TYPE_BOOL4 = 56,
  MTL2SPV_DATA_TYPE_TEXTURE = 58,
  MTL2SPV_DATA_TYPE_SAMPLER = 59,
  MTL2SPV_DATA_TYPE_POINTER = 60,
  MTL2SPV_DATA_TYPE_R8UNORM = 62,
  MTL2SPV_DATA_TYPE_R8SNORM = 63,
  MTL2SPV_DATA_TYPE_R16UNORM = 64,
  MTL2SPV_DATA_TYPE_R16SNORM = 65,
  MTL2SPV_DATA_TYPE_RG8UNORM = 66,
  MTL2SPV_DATA_TYPE_RG8SNORM = 67,
  MTL2SPV_DATA_TYPE_RG16UNORM = 68,
  MTL2SPV_DATA_TYPE_RG16SNORM = 69,
  MTL2SPV_DATA_TYPE_RGBA8UNORM = 70,
  MTL2SPV_DATA_TYPE_RGBA8UNORM_SRGB = 71,
  MTL2SPV_DATA_TYPE_RGBA8SNORM = 72,
  MTL2SPV_DATA_TYPE_RGBA16UNORM = 73,
  MTL2SPV_DATA_TYPE_RGBA16SNORM = 74,
  MTL2SPV_DATA_TYPE_RGB10A2UNORM = 75,
  MTL2SPV_DATA_TYPE_RG11B10FLOAT = 76,
  MTL2SPV_DATA_TYPE_RGB9E5FLOAT = 77,
  MTL2SPV_DATA_TYPE_RENDER_PIPELINE = 78,
  MTL2SPV_DATA_TYPE_COMPUTE_PIPELINE = 79,
  MTL2SPV_DATA_TYPE_INDIRECT_COMMANDBUFFER = 80,
  MTL2SPV_DATA_TYPE_LONG = 81,
  MTL2SPV_DATA_TYPE_LONG2 = 82,
  MTL2SPV_DATA_TYPE_LONG3 = 83,
  MTL2SPV_DATA_TYPE_LONG4 = 84,
  MTL2SPV_DATA_TYPE_ULONG = 85,
  MTL2SPV_DATA_TYPE_ULONG2 = 86,
  MTL2SPV_DATA_TYPE_ULONG3 = 87,
  MTL2SPV_DATA_TYPE_ULONG4 = 88,
  MTL2SPV_DATA_TYPE_VISIBLE_FUNCTION_TABLE = 115,
  MTL2SPV_DATA_TYPE_INTERSECTION_FUNCTION_TABLE = 116,
  MTL2SPV_DATA_TYPE_PRIMITIVE_ACCELERATION_STRUCTURE = 117,
  MTL2SPV_DATA_TYPE_INSTANCE_ACCELERATION_STRUCTURE = 118,
} mtl2spv_data_type;

typedef enum mtl2spv_argument_type {
  MTL2SPV_ARGUMENT_TYPE_BUFFER = 0,
  MTL2SPV_ARGUMENT_TYPE_THREADGROUPMEMORY = 1,
  MTL2SPV_ARGUMENT_TYPE_TEXTURE = 2,
  MTL2SPV_ARGUMENT_TYPE_SAMPLER = 3,

  MTL2SPV_ARGUMENT_TYPE_IMAGEBLOCK_DATA = 16,
  MTL2SPV_ARGUMENT_TYPE_IMAGEBLOCK = 17,

  MTL2SPV_ARGUMENT_TYPE_VISIBLE_FUNCTION_TABLE = 24,
  MTL2SPV_ARGUMENT_TYPE_PRIMITIVE_ACCELERATION_STRUCTURE = 25,
  MTL2SPV_ARGUMENT_TYPE_INSTANCE_ACCELERATION_STRUCTURE = 26,
  MTL2SPV_ARGUMENT_TYPE_INTERSECTION_FUNCTION_TABLE = 27,
} mtl2spv_argument_type;

typedef enum mtl2spv_argument_access {
  MTL2SPV_ARGUMENT_ACCESS_READ_ONLY = 0,
  MTL2SPV_ARGUMENT_ACCESS_READ_WRITE = 1,
  MTL2SPV_ARGUMENT_ACCESS_WRITE_ONLY = 2,
} mtl2spv_argument_access;

typedef enum mtl2spv_texture_type {
  MTL2SPV_TEXTURE_TYPE_1D = 0,
  MTL2SPV_TEXTURE_TYPE_1D_ARRAY = 1,
  MTL2SPV_TEXTURE_TYPE_2D = 2,
  MTL2SPV_TEXTURE_TYPE_2D_ARRAY = 3,
  MTL2SPV_TEXTURE_TYPE_2D_MULTISAMPLE = 4,
  MTL2SPV_TEXTURE_TYPE_CUBE = 5,
  MTL2SPV_TEXTURE_TYPE_CUBE_ARRAY = 6,
  MTL2SPV_TEXTURE_TYPE_3D = 7,
  MTL2SPV_TEXTURE_TYPE_2D_MULTISAMPLE_ARRAY = 8,
  MTL2SPV_TEXTURE_TYPE_TEXTURE_BUFFER = 9,
} mtl2spv_texture_type;

typedef struct mtl2spv_string {
  char *buffer;
  size_t length;
  size_t capacity;
} mtl2spv_string;

// typedef struct mtl2spv_struct_type {
//
//} mtl2spv_struct_type;

typedef struct mtl2spv_struct_member {
  struct mtl2spv_string name;
  int offset;
  mtl2spv_data_type data_type;
  int arg_index;
} mtl2spv_struct_member;

/*
class Type : public NS::Referencing<Type>
{
public:
    static class Type* alloc();
    class Type*        init();

    MTL::DataType      dataType() const;
};

class StructMember : public NS::Referencing<StructMember>
{
public:
    static class StructMember*  alloc();
    class StructMember*         init();

    NS::String*                 name() const;
    NS::UInteger                offset() const;
    MTL::DataType               dataType() const;
    class StructType*           structType();
    class ArrayType*            arrayType();
    class TextureReferenceType* textureReferenceType();
    class PointerType*          pointerType();
    NS::UInteger                argumentIndex() const;
};

class StructType : public NS::Referencing<StructType, Type>
{
public:
    static class StructType* alloc();
    class StructType*        init();

    NS::Array*               members() const;
    class StructMember*      memberByName(const NS::String* name);
};

class ArrayType : public NS::Referencing<ArrayType, Type>
{
public:
    static class ArrayType*     alloc();
    class ArrayType*            init();

    MTL::DataType               elementType() const;
    NS::UInteger                arrayLength() const;
    NS::UInteger                stride() const;
    NS::UInteger                argumentIndexStride() const;
    class StructType*           elementStructType();
    class ArrayType*            elementArrayType();
    class TextureReferenceType* elementTextureReferenceType();
    class PointerType*          elementPointerType();
};

class PointerType : public NS::Referencing<PointerType, Type>
{
public:
    static class PointerType* alloc();
    class PointerType*        init();

    MTL::DataType             elementType() const;
    MTL::ArgumentAccess       access() const;
    NS::UInteger              alignment() const;
    NS::UInteger              dataSize() const;
    bool                      elementIsArgumentBuffer() const;
    class StructType*         elementStructType();
    class ArrayType*          elementArrayType();
};

class TextureReferenceType : public NS::Referencing<TextureReferenceType, Type>
{
public:
    static class TextureReferenceType* alloc();
    class TextureReferenceType*        init();

    MTL::DataType                      textureDataType() const;
    MTL::TextureType                   textureType() const;
    MTL::ArgumentAccess                access() const;
    bool                               isDepthTexture() const;
};
*/

typedef struct mtl2spv_arg_buffer {
  int alignment;
  int data_size;
  mtl2spv_data_type data_type;
  // TODO:
  // buffer struct type
  // buffer pointer type
} mtl2spv_arg_buffer;

typedef struct mtl2spv_arg_texture {
  mtl2spv_texture_type texture_type;
  mtl2spv_data_type data_type;
  int is_depth;
} mtl2spv_arg_texture;

typedef struct mtl2spv_arg_threadgroup_memory {
  int alignment;
  int data_size;
} mtl2spv_arg_threadgroup_memory;

typedef struct mtl2spv_argument {
  struct mtl2spv_string name;
  mtl2spv_argument_type type;
  mtl2spv_argument_access access;
  int index;
  int array_len;
  union {
    mtl2spv_arg_buffer buffer;
    mtl2spv_arg_texture texture;
    mtl2spv_arg_threadgroup_memory threadgroup_memory;
  };
} mtl2spv_argument;

typedef struct mtl2spv_function {
  struct mtl2spv_string name;
  mtl2spv_shader_type type;
  void *module;
  void *buffer;
} mtl2spv_function;

typedef struct mtl2spv_options {
  unsigned int vk_version; // 1_0 or 1_1
  unsigned int device_caps;
} mtl2spv_options;

typedef struct metallib {
} * metallib_t;

METALIB2SPIRV_API mtl2spv_error metallib_open(const void *bytes,
                                              size_t num_bytes,
                                              metallib_t *lib);
METALIB2SPIRV_API void metallib_free(metallib_t lib);
METALIB2SPIRV_API int metallib_function_count(metallib_t lib);
METALIB2SPIRV_API const char *metallib_function_name(metallib_t lib, int index);
METALIB2SPIRV_API const mtl2spv_function *metallib_function(metallib_t lib,
                                                            int index);
METALIB2SPIRV_API mtl2spv_shader_type metallib_function_type(metallib_t lib,
                                                             int index);
METALIB2SPIRV_API void metallib2spv(metallib_t lib,
                                    const mtl2spv_options *options, int index);
METALIB2SPIRV_API void metallib2spv2(metallib_t lib,
                                     const mtl2spv_options *options,
                                     const char *entry_name);
METALIB2SPIRV_API const uint32_t *metallib_spirv_data(metallib_t lib,
                                                      int index,
                                                      size_t *word_count);
METALIB2SPIRV_API size_t metallib_spirv_word_count(metallib_t lib, int index);
METALIB2SPIRV_API const char *metallib_last_error(metallib_t lib);

#ifdef __cplusplus
}
#endif

#endif
