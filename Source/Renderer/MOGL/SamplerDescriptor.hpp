#pragma once
#include <stdint.h>
#include <GL/glew.h>

enum class SamplerAddressMode : uint32_t {
    CLAMP           = GL_CLAMP,
    CLAMP_TO_EDGE   = GL_CLAMP_TO_EDGE,
    CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
    REPEAT          = GL_REPEAT
};

enum class SamplerMinMagFilter : uint32_t {
    LINEAR = GL_LINEAR,
    NEAREST = GL_NEAREST,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
    LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST
};

struct SamplerDescriptor {
    SamplerAddressMode      rAddressMode;
    SamplerAddressMode      sAddressMode;
    SamplerAddressMode      tAddressMode;
    SamplerMinMagFilter     minFilter;
    SamplerMinMagFilter     magFilter;
//    uint32_t    mipFilter;
    uint32_t    lodMinClamp;
    uint32_t    lodMaxClamp;
    float       maxAnisotropy;
    bool        normalizedCoordinates;
};