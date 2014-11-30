#pragma once
#include "Config/Prerequisities.h"
#include <GL/glew.h>

enum class TextureType : uint32_t {
    TEXTURE_1D = GL_TEXTURE_1D,
    TEXTURE_2D = GL_TEXTURE_2D,
    TEXTURE_3D = GL_TEXTURE_3D,
    TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY
};

enum class PixelFormat : uint32_t {

};

struct TextureDescriptor {
    TextureType textureType;
    PixelFormat pixelFormat;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevelCount;
    uint32_t arrayLength;
    uint32_t sampleCount;

//    MTLResourceOptions cache optimize
};