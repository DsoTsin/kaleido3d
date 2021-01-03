#pragma once
#include <ngfx.h>
#include "volk.h"

namespace vulkan {

inline VkFormat pixelFormatToVk(ngfx::PixelFormat fmt) {
    switch (fmt) {
    case ngfx::PixelFormat::R8Unorm:            return VK_FORMAT_R8_UNORM;
    case ngfx::PixelFormat::R8Unorm_sRGB:       return VK_FORMAT_R8_SRGB;
    case ngfx::PixelFormat::R8Snorm:            return VK_FORMAT_R8_SNORM;
    case ngfx::PixelFormat::R8Uint:             return VK_FORMAT_R8_UINT;
    case ngfx::PixelFormat::R8Sint:             return VK_FORMAT_R8_SINT;
    case ngfx::PixelFormat::R16Unorm:           return VK_FORMAT_R16_UNORM;
    case ngfx::PixelFormat::R16Snorm:           return VK_FORMAT_R16_SNORM;
    case ngfx::PixelFormat::R16Uint:            return VK_FORMAT_R16_UINT;
    case ngfx::PixelFormat::R16Sint:            return VK_FORMAT_R16_SINT;
    case ngfx::PixelFormat::R16Float:           return VK_FORMAT_R16_SFLOAT;
    case ngfx::PixelFormat::RG8Unorm:           return VK_FORMAT_R8G8_UNORM;
    case ngfx::PixelFormat::RG8Unorm_sRGB:      return VK_FORMAT_R8G8_SRGB;
    case ngfx::PixelFormat::RG8Snorm:           return VK_FORMAT_R8G8_SNORM;
    case ngfx::PixelFormat::RG8Uint:            return VK_FORMAT_R8G8_UINT;
    case ngfx::PixelFormat::RG8Sint:            return VK_FORMAT_R8G8_SINT;
    case ngfx::PixelFormat::R32Uint:            return VK_FORMAT_R32_UINT;
    case ngfx::PixelFormat::R32Sint:            return VK_FORMAT_R32_SINT;
    case ngfx::PixelFormat::R32Float:           return VK_FORMAT_R32_SFLOAT;
    case ngfx::PixelFormat::RG16Unorm:          return VK_FORMAT_R16G16_UNORM;
    case ngfx::PixelFormat::RG16Snorm:          return VK_FORMAT_R16G16_SNORM;
    case ngfx::PixelFormat::RG16Uint:           return VK_FORMAT_R16G16_UINT;
    case ngfx::PixelFormat::RG16Sint:           return VK_FORMAT_R16G16_SINT;
    case ngfx::PixelFormat::RG16Float:          return VK_FORMAT_R16G16_SFLOAT;
    case ngfx::PixelFormat::RGBA8Unorm:         return VK_FORMAT_R8G8B8A8_UNORM;
    case ngfx::PixelFormat::RGBA8Unorm_sRGB:    return VK_FORMAT_R8G8B8A8_SRGB;
    case ngfx::PixelFormat::RGBA8Snorm:         return VK_FORMAT_R8G8B8A8_SNORM;
    case ngfx::PixelFormat::RGBA8Uint:          return VK_FORMAT_R8G8B8A8_UINT;
    case ngfx::PixelFormat::RGBA8Sint:          return VK_FORMAT_R8G8B8A8_SINT;
    case ngfx::PixelFormat::BGRA8Unorm:         return VK_FORMAT_B8G8R8A8_UNORM;
    case ngfx::PixelFormat::BGRA8Unorm_sRGB:    return VK_FORMAT_B8G8R8A8_SRGB;
    case ngfx::PixelFormat::RG11B10Float:       return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case ngfx::PixelFormat::RGB9E5Float:        return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case ngfx::PixelFormat::RG32Uint:           return VK_FORMAT_R32G32_UINT;
    case ngfx::PixelFormat::RG32Sint:           return VK_FORMAT_R32G32_SINT;
    case ngfx::PixelFormat::RG32Float:          return VK_FORMAT_R32G32_SFLOAT;
    case ngfx::PixelFormat::RGBA16Unorm:        return VK_FORMAT_R16G16B16A16_UNORM;
    case ngfx::PixelFormat::RGBA16Snorm:        return VK_FORMAT_R16G16B16A16_SNORM;
    case ngfx::PixelFormat::RGBA16Uint:         return VK_FORMAT_R16G16B16A16_UINT;
    case ngfx::PixelFormat::RGBA16Sint:         return VK_FORMAT_R16G16B16A16_SINT;
    case ngfx::PixelFormat::RGBA16Float:        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ngfx::PixelFormat::RGBA32Uint:         return VK_FORMAT_R32G32B32A32_UINT;
    case ngfx::PixelFormat::RGBA32Sint:         return VK_FORMAT_R32G32B32A32_SINT;
    case ngfx::PixelFormat::RGBA32Float:        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ngfx::PixelFormat::Depth16Unorm:       return VK_FORMAT_D16_UNORM;
    case ngfx::PixelFormat::Depth32Float:       return VK_FORMAT_D32_SFLOAT;
    case ngfx::PixelFormat::Stencil8:           return VK_FORMAT_S8_UINT;
    case ngfx::PixelFormat::Depth24UnormStencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
    case ngfx::PixelFormat::Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case ngfx::PixelFormat::RGB8Unorm_ETC2:     return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case ngfx::PixelFormat::RGB8_sRGB_ETC2:     return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case ngfx::PixelFormat::RGBA8Unorm_ETC2:    return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case ngfx::PixelFormat::RGBA8_sRGB_ETC2:    return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
    default:                                    return VK_FORMAT_UNDEFINED;
    }
}

inline ngfx::PixelFormat vkFormatToPixel(VkFormat fmt) {
    switch (fmt) {
    case VK_FORMAT_R8G8B8A8_UNORM:      return ngfx::PixelFormat::RGBA8Unorm;
    case VK_FORMAT_R8G8B8A8_SRGB:       return ngfx::PixelFormat::RGBA8Unorm_sRGB;
    case VK_FORMAT_B8G8R8A8_UNORM:      return ngfx::PixelFormat::BGRA8Unorm;
    case VK_FORMAT_B8G8R8A8_SRGB:       return ngfx::PixelFormat::BGRA8Unorm_sRGB;
    case VK_FORMAT_R16G16B16A16_SFLOAT:  return ngfx::PixelFormat::RGBA16Float;
    case VK_FORMAT_R32G32B32A32_SFLOAT:  return ngfx::PixelFormat::RGBA32Float;
    case VK_FORMAT_D16_UNORM:           return ngfx::PixelFormat::Depth16Unorm;
    case VK_FORMAT_D32_SFLOAT:          return ngfx::PixelFormat::Depth32Float;
    case VK_FORMAT_D24_UNORM_S8_UINT:   return ngfx::PixelFormat::Depth24UnormStencil8;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:  return ngfx::PixelFormat::Depth32FloatStencil8;
    default:                            return ngfx::PixelFormat::Invalid;
    }
}

inline VkFormat vertexFormatToVk(ngfx::VertexFormat fmt) {
    switch (fmt) {
    case ngfx::VertexFormat::UChar:     return VK_FORMAT_R8_UINT;
    case ngfx::VertexFormat::UChar2:    return VK_FORMAT_R8G8_UINT;
    case ngfx::VertexFormat::UChar3:    return VK_FORMAT_R8G8B8_UINT;
    case ngfx::VertexFormat::UChar4:    return VK_FORMAT_R8G8B8A8_UINT;
    case ngfx::VertexFormat::Char:      return VK_FORMAT_R8_SINT;
    case ngfx::VertexFormat::Char2:     return VK_FORMAT_R8G8_SINT;
    case ngfx::VertexFormat::Char3:     return VK_FORMAT_R8G8B8_SINT;
    case ngfx::VertexFormat::Char4:     return VK_FORMAT_R8G8B8A8_SINT;
    case ngfx::VertexFormat::UShort:    return VK_FORMAT_R16_UINT;
    case ngfx::VertexFormat::UShort2:   return VK_FORMAT_R16G16_UINT;
    case ngfx::VertexFormat::UShort3:   return VK_FORMAT_R16G16B16_UINT;
    case ngfx::VertexFormat::UShort4:   return VK_FORMAT_R16G16B16A16_UINT;
    case ngfx::VertexFormat::Short:     return VK_FORMAT_R16_SINT;
    case ngfx::VertexFormat::Short2:    return VK_FORMAT_R16G16_SINT;
    case ngfx::VertexFormat::Short3:    return VK_FORMAT_R16G16B16_SINT;
    case ngfx::VertexFormat::Short4:    return VK_FORMAT_R16G16B16A16_SINT;
    case ngfx::VertexFormat::Half:      return VK_FORMAT_R16_SFLOAT;
    case ngfx::VertexFormat::Half2:     return VK_FORMAT_R16G16_SFLOAT;
    case ngfx::VertexFormat::Half3:     return VK_FORMAT_R16G16B16_SFLOAT;
    case ngfx::VertexFormat::Half4:     return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ngfx::VertexFormat::Float:     return VK_FORMAT_R32_SFLOAT;
    case ngfx::VertexFormat::Float2:    return VK_FORMAT_R32G32_SFLOAT;
    case ngfx::VertexFormat::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
    case ngfx::VertexFormat::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ngfx::VertexFormat::UInt:      return VK_FORMAT_R32_UINT;
    case ngfx::VertexFormat::UInt2:     return VK_FORMAT_R32G32_UINT;
    case ngfx::VertexFormat::UInt3:     return VK_FORMAT_R32G32B32_UINT;
    case ngfx::VertexFormat::UInt4:     return VK_FORMAT_R32G32B32A32_UINT;
    case ngfx::VertexFormat::Int:       return VK_FORMAT_R32_SINT;
    case ngfx::VertexFormat::Int2:      return VK_FORMAT_R32G32_SINT;
    case ngfx::VertexFormat::Int3:      return VK_FORMAT_R32G32B32_SINT;
    case ngfx::VertexFormat::Int4:      return VK_FORMAT_R32G32B32A32_SINT;
    case ngfx::VertexFormat::Int1010102Norm:  return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
    case ngfx::VertexFormat::UInt1010102Norm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    default:                            return VK_FORMAT_UNDEFINED;
    }
}

inline VkFilter filterModeToVk(ngfx::FilterMode mode) {
    return mode == ngfx::FilterMode::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
}

inline VkSamplerMipmapMode mipFilterToVk(ngfx::FilterMode mode) {
    return mode == ngfx::FilterMode::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

inline VkSamplerAddressMode addressModeToVk(ngfx::SamplerAddressMode mode) {
    switch (mode) {
    case ngfx::SamplerAddressMode::Wrap:       return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case ngfx::SamplerAddressMode::Mirror:     return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case ngfx::SamplerAddressMode::Clamp:      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case ngfx::SamplerAddressMode::Border:     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case ngfx::SamplerAddressMode::MirrorOnce: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:                                   return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

inline VkCompareOp compareOpToVk(ngfx::ComparisonFunction fn) {
    switch (fn) {
    case ngfx::ComparisonFunction::Never:        return VK_COMPARE_OP_NEVER;
    case ngfx::ComparisonFunction::Less:         return VK_COMPARE_OP_LESS;
    case ngfx::ComparisonFunction::Equal:        return VK_COMPARE_OP_EQUAL;
    case ngfx::ComparisonFunction::LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ngfx::ComparisonFunction::Greater:      return VK_COMPARE_OP_GREATER;
    case ngfx::ComparisonFunction::NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
    case ngfx::ComparisonFunction::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ngfx::ComparisonFunction::Always:       return VK_COMPARE_OP_ALWAYS;
    default:                                     return VK_COMPARE_OP_NEVER;
    }
}

inline VkStencilOp stencilOpToVk(ngfx::StencilOperation op) {
    switch (op) {
    case ngfx::StencilOperation::Keep:           return VK_STENCIL_OP_KEEP;
    case ngfx::StencilOperation::Zero:           return VK_STENCIL_OP_ZERO;
    case ngfx::StencilOperation::Replace:        return VK_STENCIL_OP_REPLACE;
    case ngfx::StencilOperation::IncrementClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case ngfx::StencilOperation::DecrementClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case ngfx::StencilOperation::Invert:         return VK_STENCIL_OP_INVERT;
    case ngfx::StencilOperation::IncrementWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case ngfx::StencilOperation::DecrementWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default:                                     return VK_STENCIL_OP_KEEP;
    }
}

inline VkBlendFactor blendFactorToVk(ngfx::BlendFactor f) {
    switch (f) {
    case ngfx::BlendFactor::Zero:                return VK_BLEND_FACTOR_ZERO;
    case ngfx::BlendFactor::One:                 return VK_BLEND_FACTOR_ONE;
    case ngfx::BlendFactor::SrcColor:            return VK_BLEND_FACTOR_SRC_COLOR;
    case ngfx::BlendFactor::OneMinusSrcColor:    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case ngfx::BlendFactor::SrcAlpha:            return VK_BLEND_FACTOR_SRC_ALPHA;
    case ngfx::BlendFactor::OneMinusSrcAlpha:    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case ngfx::BlendFactor::DestColor:           return VK_BLEND_FACTOR_DST_COLOR;
    case ngfx::BlendFactor::OneMinusDestColor:   return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case ngfx::BlendFactor::DestAlpha:           return VK_BLEND_FACTOR_DST_ALPHA;
    case ngfx::BlendFactor::OneMinusDestAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    default:                                     return VK_BLEND_FACTOR_ZERO;
    }
}

inline VkBlendOp blendOpToVk(ngfx::BlendOperation op) {
    switch (op) {
    case ngfx::BlendOperation::Add:    return VK_BLEND_OP_ADD;
    case ngfx::BlendOperation::Sub:    return VK_BLEND_OP_SUBTRACT;
    case ngfx::BlendOperation::RevSub: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case ngfx::BlendOperation::Min:    return VK_BLEND_OP_MIN;
    case ngfx::BlendOperation::Max:    return VK_BLEND_OP_MAX;
    default:                           return VK_BLEND_OP_ADD;
    }
}

inline VkPolygonMode fillModeToVk(ngfx::FillMode m) {
    return m == ngfx::FillMode::Line ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
}

inline VkCullModeFlags cullModeToVk(ngfx::CullMode m) {
    switch (m) {
    case ngfx::CullMode::None:  return VK_CULL_MODE_NONE;
    case ngfx::CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
    case ngfx::CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
    default:                    return VK_CULL_MODE_NONE;
    }
}

inline VkPrimitiveTopology primitiveToVk(ngfx::PrimitiveType p) {
    switch (p) {
    case ngfx::PrimitiveType::Points:         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case ngfx::PrimitiveType::Lines:          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case ngfx::PrimitiveType::LineStrips:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case ngfx::PrimitiveType::Triangles:      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case ngfx::PrimitiveType::TriangleStrips: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    default:                                  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

inline VkIndexType indexTypeToVk(ngfx::IndexType t) {
    return t == ngfx::IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
}

inline VkSampleCountFlagBits multisampleToVk(ngfx::MultisampleFlags ms) {
    switch (ms) {
    case ngfx::MultisampleFlags::MS1X:  return VK_SAMPLE_COUNT_1_BIT;
    case ngfx::MultisampleFlags::MS2X:  return VK_SAMPLE_COUNT_2_BIT;
    case ngfx::MultisampleFlags::MS4X:  return VK_SAMPLE_COUNT_4_BIT;
    case ngfx::MultisampleFlags::MS8X:  return VK_SAMPLE_COUNT_8_BIT;
    case ngfx::MultisampleFlags::MS16X: return VK_SAMPLE_COUNT_16_BIT;
    default:                            return VK_SAMPLE_COUNT_1_BIT;
    }
}

inline VkImageType texDimToVk(ngfx::TextureDim d) {
    switch (d) {
    case ngfx::TextureDim::Tex1D: return VK_IMAGE_TYPE_1D;
    case ngfx::TextureDim::Tex2D: return VK_IMAGE_TYPE_2D;
    case ngfx::TextureDim::Tex3D: return VK_IMAGE_TYPE_3D;
    default:                      return VK_IMAGE_TYPE_2D;
    }
}

inline VkImageViewType texDimToViewType(ngfx::TextureDim d, uint32_t layers) {
    switch (d) {
    case ngfx::TextureDim::Tex1D: return layers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    case ngfx::TextureDim::Tex2D: return layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    case ngfx::TextureDim::Tex3D: return VK_IMAGE_VIEW_TYPE_3D;
    default:                      return VK_IMAGE_VIEW_TYPE_2D;
    }
}

inline bool isDepthStencilFormat(ngfx::PixelFormat fmt) {
    return fmt == ngfx::PixelFormat::Depth16Unorm ||
           fmt == ngfx::PixelFormat::Depth32Float ||
           fmt == ngfx::PixelFormat::Stencil8 ||
           fmt == ngfx::PixelFormat::Depth24UnormStencil8 ||
           fmt == ngfx::PixelFormat::Depth32FloatStencil8;
}

inline bool hasStencil(ngfx::PixelFormat fmt) {
    return fmt == ngfx::PixelFormat::Stencil8 ||
           fmt == ngfx::PixelFormat::Depth24UnormStencil8 ||
           fmt == ngfx::PixelFormat::Depth32FloatStencil8;
}

inline VkImageAspectFlags imageAspect(ngfx::PixelFormat fmt) {
    if (fmt == ngfx::PixelFormat::Stencil8)
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    if (fmt == ngfx::PixelFormat::Depth24UnormStencil8 || fmt == ngfx::PixelFormat::Depth32FloatStencil8)
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    if (fmt == ngfx::PixelFormat::Depth16Unorm || fmt == ngfx::PixelFormat::Depth32Float)
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

inline VkAttachmentLoadOp loadActionToVk(ngfx::LoadAction a) {
    switch (a) {
    case ngfx::LoadAction::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
    case ngfx::LoadAction::Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case ngfx::LoadAction::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default:                         return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

inline VkAttachmentStoreOp storeActionToVk(ngfx::StoreAction a) {
    switch (a) {
    case ngfx::StoreAction::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
    case ngfx::StoreAction::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    default:                          return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

inline VkVertexInputRate inputRateToVk(ngfx::VertexInputRate r) {
    return r == ngfx::VertexInputRate::PerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
}

} // namespace vulkan
