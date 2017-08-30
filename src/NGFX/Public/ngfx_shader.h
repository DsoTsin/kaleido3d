#ifndef __ngfx_shader_20180511__
#define __ngfx_shader_20180511__
#pragma once

#if _WIN32
#if defined(NGFX_SHADER_BUILD)
#if defined(NGFX_SHADER_BUILD_SHARED_LIB)
#define NGFX_SHADER_API __declspec(dllexport)
#else
#define NGFX_SHADER_API __declspec(dllimport)
#endif
#else
#define NGFX_SHADER_API     
#endif
#else
#define NGFX_SHADER_API __attribute__((visibility("default"))) 
#endif
#include "ngfx.h"

namespace ngfx {
    enum class _ShaderProfile : int32_t
    {
        SM3_0 = 30, // Instancing
        SM3_1 = 31, // Compute
        SM5_0 = 50, // Compute, Tess
        SM6_0 = 60, // Add Raytracing
    };

#define MAKE_SHADER_FORMAT(a,b,c,d) uint32_t(a)<<24 | uint32_t(b) << 16 | uint32_t(c) << 8 | uint32_t(d)

    enum class _ShaderFormat : uint32_t
    {
        DXBC = MAKE_SHADER_FORMAT('D', 'X', 'B', 'C'), // SM5
        DXIL = MAKE_SHADER_FORMAT('D', 'X', 'I', 'L'), // SM6
        SPV1 = MAKE_SHADER_FORMAT('S', 'P', 'V', '1'), // VK
        HLSL = MAKE_SHADER_FORMAT('H', 'L', 'S', 'L'), // D3D
        GLSL = MAKE_SHADER_FORMAT('G', 'L', 'S', 'L'), // GL
        GLBC = MAKE_SHADER_FORMAT('G', 'L', 'B', 'C'), // GL Binary
        MTBC = MAKE_SHADER_FORMAT('M', 'T', 'B', 'C'), // Metal
        MTSL = MAKE_SHADER_FORMAT('M', 'T', 'S', 'L'), // Metal
    };
    /*
    * OpenGL ES 3.+: BlockBuffer, SamplerTexture
    * Metal        : Buffer, Texture
    * Direct3D     : CBV, SRV, UAV
    * Vulkan       : Buffer, StorageImage, SamplerImage, TexelBuffer
    */
    class NGFX_SHADER_API SerializedShaderVariable
    {
    public:
        enum Type : uint8_t {
            VT_Buffer = 1, // structs, matrixes, vectors, except constants
            VT_Texture = 2, // texture{123d}{array}
            VT_Sampler = 3,
        };
        enum Access : uint8_t {
            VA_Read = 1,
            VA_Write = 2,
            VA_ReadWrite = 3,
        };
        enum Dimension : uint8_t {
            VTD_Tex1D,
            VTD_Tex2D,
            VTD_Tex2DMS,
            VTD_Tex2DArray,
            VTD_Tex3D,
            VTD_Tex3DArray,
            VTD_TexCube,
            VTD_Buffer,
        };
        SerializedShaderVariable();
        ~SerializedShaderVariable();

        void Serialize(void* InOutData, size_t Length, bool In = false);

    private:
        Type            m_Type;
        Access          m_Access;
        int32_t         m_Index;
        int32_t         m_ArrayLength;
        Dimension       m_TexDim;
        int32_t         m_BufferAlignment;
        //ShaderStageBit  m_Stage : 8;
        int32_t         m_NameLength;
        char*           m_Name;
    };
	class NGFX_SHADER_API SerializedShader
	{
	public:
        SerializedShader();
        ~SerializedShader();

        void Serialize(void* InOutData, size_t Length, bool In = false);

    private:
        // Length of shader
        int32_t         m_Length;
        int32_t         m_Version;
        _ShaderProfile  m_Profile;
        // DXBC/SPV1/DXIL/MTBC/GLSL/GLBC/ESSL/MTSL
        _ShaderFormat   m_Format;
        // Shader HashCode
        uint64_t        m_Hash;
        // Shader Type
        //ShaderStageBit  m_Stage : 8;
        uint32_t        m_EntryPointNameLength;
        char*           m_EntryPointName;
        uint32_t        m_CodeLength;
        int8_t*         m_Code;
	};
    class NGFX_SHADER_API SerializedShaderLibrary
    {
    public:
        SerializedShaderLibrary();
        ~SerializedShaderLibrary();

        void Serialize(void* InOutData, size_t Length, bool In = false);
    private:
        int32_t         m_Length;
        int32_t         m_Version;
        uint64_t        m_Hash;
        //ShaderStageBit  m_Stages : 8;
        uint32_t        m_NumVariables;
        SerializedShaderVariable* m_Variables;
        SerializedShader* m_Shaders;
    };
}

#endif