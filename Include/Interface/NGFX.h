#ifndef __NGFX_h__
#define __NGFX_h__
#include "../KTL/String.hpp"
#include "../KTL/SharedPtr.hpp"
#include "../Math/kGeometry.hpp"


// for c

enum NGFXRHIType
{
  NGFX_RHI_VULKAN,
  NGFX_RHI_METAL,
  NGFX_RHI_D3D12,
  NGFX_RHI_OPENGL,
};

enum NGFXCommandType
{
  NGFX_COMMAND_BUNDLE,
  NGFX_COMMAND_GRAPHICS,
  NGFX_COMMAND_COMPUTE,
};

enum NGFXCommandReuseType
{
  NGFX_COMMAND_USAGE_ONE_SHOT,
  NGFX_COMMAND_USAGE_REUSABLE
};

enum NGFXPipelineType
{
  NGFX_PIPELINE_Compute,
  NGFX_PIPELINE_GRAPHICS
};

enum NGFXPixelFormat
{
  NGFX_PIXEL_FORMAT_RGBA16_UINT,
  NGFX_PIXEL_FORMAT_RGBA32_FLOAT,
  NGFX_PIXEL_FORMAT_RGBA8_UNORM,
  NGFX_PIXEL_FORMAT_RGBA8_UNORM_SRGB,
  NGFX_PIXEL_FORMAT_R11_G11_B10_FLOAT,
  NGFX_PIXEL_FORMAT_D32_FLOAT,
  NGFX_PIXEL_FORMAT_RGB32_FLOAT,
  NGFX_PIXEL_FORMAT_RGB8_UNORM,
  NGFX_PIXEL_FORMAT_BGRA8_UNORM, // Apple Metal Layer uses it as default pixel format
  NGFX_PIXEL_FORMAT_BGRA8_UNORM_SRGB,
  NGFX_PIXEL_FORMAT_RGBA16_FLOAT,
  NGFX_PIXEL_FORMAT_D24_S8,
  NGFX_PIXEL_FORMAT_NUM,
};

enum NGFXVertexFormat
{
  NGFX_VERTEX_FORMAT_FLOAT1X32,
  NGFX_VERTEX_FORMAT_FLOAT2X32,
  NGFX_VERTEX_FORMAT_FLOAT3X32,
  NGFX_VERTEX_FORMAT_FLOAT4X32,
  NGFX_VERTEX_FORMAT_NUM
};

enum NGFXVertexInputRate
{
  NGFX_VERTEX_INPUT_RATE_PER_VERTEX,
  NGFX_VERTEX_INPUT_RATE_PER_INSTANCE
};

enum NGFXMultiSampleFlag
{
  NGFX_MULTI_SAMPLE_1X,
  NGFX_MULTI_SAMPLE_2X,
  NGFX_MULTI_SAMPLE_4X,
  NGFX_MULTI_SAMPLE_8X,
  NGFX_MULTI_SAMPLE_16X,
  NGFX_MULTI_SAMPLE_32X,
};

enum NGFXPrimitiveType
{
  NGFX_PRIMITIVE_POINTS,
  NGFX_PRIMITIVE_LINES,
  NGFX_PRIMITIVE_TRIANGLES,
  NGFX_PRIMITIVE_TRIANGLE_STRIP,
  NGFX_PRIMITIVE_NUM
};

enum NGFXShaderType
{
  NGFX_SHADER_TYPE_FRAGMENT,
  NGFX_SHADER_TYPE_VERTEX,
  NGFX_SHADER_TYPE_GEOMETRY,
  NGFX_SHADER_TYPE_HULL,
  NGFX_SHADER_TYPE_DOMAIN,
  NGFX_SHADER_TYPE_COMPUTE,
};

enum NGFXLoadAction
{
  NGFX_LOAD_ACTION_LOAD,
  NGFX_LOAD_ACTION_CLEAR,
  NGFX_LOAD_ACTION_DONT_CARE
};

enum NGFXStoreAction
{
  NGFX_STORE_ACTION_STORE,
  NGFX_STORE_ACTION_DONT_CARE
};

enum NGFXBlendOperation
{
  NGFX_BLEND_OP_ADD,
  NGFX_BLEND_OP_SUB,
  NGFX_BLEND_OP_NUM
};

enum NGFXBlendFactor
{
  NGFX_BLEND_FACTOR_ZERO,
  NGFX_BLEND_FACTOR_ONE,
  NGFX_BLEND_FACTOE_SRC_COLOR,
  NGFX_BLEND_FACTOR_DEST_COLOR,
  NGFX_BLEND_FACTOR_SRC_ALPHA,
  NGFX_BLEND_FACTOR_DEST_ALPHA,
  NGFX_BLEND_FACTOR_NUM
};

enum NGFXFillMode
{
  NGFX_FILL_MODE_WIRE_FRAME,
  NGFX_FILL_MODE_SOLID,
  NGFX_FILL_MODE_NUM
};

enum NGFXCullMode
{
  NGFX_CULL_MODE_NONE,
  NGFX_CULL_MODE_FRONT,
  NGFX_CULL_MODE_BACK,
  NGFX_CULL_MODE_NUM
};

enum NGFXStencilOp
{
  NGFX_STENCIL_OP_KEEP,
  NGFX_STENCIL_OP_ZERO,
  NGFX_STENCIL_OP_REPLACE,
  NGFX_STENCIL_OP_INVERT,
  NGFX_STENCIL_OP_INCREMENT,
  NGFX_STENCIL_OP_DECREMENT,
  NGFX_STENCIL_OP_NUM
};

enum NGFXComparisonFunc
{
  NGFX_COMPARISON_FUNCTION_NEVER,
  NGFX_COMPARISON_FUNCTION_LESS,
  NGFX_COMPARISON_FUNCTION_EQUAL,
  NGFX_COMPARISON_FUNCTION_LESS_EQUAL,
  NGFX_COMPARISON_FUNCTION_GREATER,
  NGFX_COMPARISON_FUNCTION_NOT_EQUAL,
  NGFX_COMPARISON_FUNCTION_GREATER_EQUAL,
  NGFX_COMPARISON_FUNCTION_ALWAYS,
  NGFX_COMPARISON_FUNCTION_NUM
};

enum NGFXDepthWriteMask
{
  NGFX_DEPTH_WRITE_MASK_ZERO,
  NGFX_DEPTH_WRITE_MASK_ALL,
  NGFX_DEPTH_WRITE_MASK_NUM
};

enum NGFXFilterMethod
{
  NGFX_FILTER_METHOD_POINT, // Nearest
  NGFX_FILTER_METHOD_LINEAR,
  NGFX_FILTER_METHOD_NUM
};

enum NGFXFilterReductionType
{
  EFRT_Standard,
  EFRT_Comparison, // all three other filter should be linear
  EFRT_Minimum,
  EFRT_Maximum,
  FilterReductionTypeNum
};

enum NGFXAddressMode
{
  NGFX_ADDRESS_MODE_WRAP,
  NGFX_ADDRESS_MODE_MIRROR, // Repeat
  NGFX_ADDRESS_MODE_CLAMP,
  NGFX_ADDRESS_MODE_BORDER,
  NGFX_ADDRESS_MODE_MIRROR_ONCE,
  NGFX_ADDRESS_MODE_NUM
};


/*
VK_IMAGE_LAYOUT_GENERAL / D3D12_RESOURCE_STATE_COMMON
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL / D3D12_RESOURCE_STATE_RENDER_TARGET
VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL /
D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ
VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL /
D3D12_RESOURCE_STATE_DEPTH_READ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL /
D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
D3D12_RESOURCE_STATE_UNORDERED_ACCESS VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL /
D3D12_RESOURCE_STATE_COPY_SOURCE VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL /
D3D12_RESOURCE_STATE_COPY_DEST VK_IMAGE_LAYOUT_PRESENT_SRC_KHR /
D3D12_RESOURCE_STATE_PRESENT
D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER |
D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
*/
enum NGFXResourceState
{
  NGFX_RESOURCE_STATE_COMMON,
  NGFX_RESOURCE_STATE_PRESENT, // D3D12_RESOURCE_STATE_PRESENT/VK_IMAGE_LAYOUT_PRESENT_SRC_KHR?
  NGFX_RESOURCE_STATE_RENDER_TARGET, // D3D12_RESOURCE_STATE_RENDER_TARGET/VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL?
  NGFX_RESOURCE_STATE_SHADER_RESOURCE,
  NGFX_RESOURCE_STATE_TRANSFER_DST,
  NGFX_RESOURCE_STATE_TRANSFER_SRC,
  NGFX_RESOURCE_STATE_DEPTH_STENCIL,
  NGFX_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
  NGFX_RESOURCE_STATE_UNORDERED_ACCESS,
  NGFX_RESOURCE_STATE_UNKNOWN,
  NGFX_RESOURCE_STATE_NUM
};

enum NGFXResourceViewTypeBits
{
  NGFX_RESOURCE_VIEW_UNDEFINED = 0,
  NGFX_RESOURCE_VERTEX_BUFFER_VIEW = 1, // For VertexBuffer
  NGFX_RESOURCE_INDEX_BUFFER_VIEW = 2, // For IndexBuffer
  NGFX_RESOURCE_CONSTANT_BUFFER_VIEW = 4, // For ConstantBuffer,
  NGFX_RESOURCE_SHADER_RESOURCE_VIEW = 8, // For Texture
  NGFX_RESOURCE_UNORDERED_ACCESS_VIEW = 16, // For Buffer
  NGFX_RESOURCE_RENDER_TARGET_VIEW = 32,
  NGFX_RESOURCE_DEPTH_STENCIL_VIEW = 64,
  /*
  EGVT_Sampler = 128,
  EGVT_SOV,
  */
};

enum NGFXResourceType
{
  NGFX_TEXTURE_1D,
  NGFX_TEXTURE_1D_ARRAY,
  NGFX_TEXTURE_2D,
  NGFX_TEXTURE_2D_ARRAY,
  NGFX_TEXTURE_2DMS,
  NGFX_TEXTURE_2DMS_ARRAY,
  NGFX_TEXTURE_3D,
  NGFX_TEXTURE_CUBE,
  NGFX_BUFFER,
  NGFX_RESOURCE_TYPE_NUM
};

enum NGFXViewDimension
{
  NGFX_VIEW_DIMENSION_BUFFER,
  NGFX_VIEW_DIMENSION_TEXTURE_1D,
  NGFX_VIEW_DIMENSION_TEXTURE_2D,
  NGFX_VIEW_DIMENSION_TEXTURE_3D,
  NGFX_VIEW_DIMENSION_NUM
};

enum NGFXResourceAccessFlag
{
  NGFX_ACCESS_READ = 0x1,
  NGFX_ACCESS_WRITE = 0x1 << 1,
  NGFX_ACCESS_READ_AND_WRITE = 0x3,
  NGFX_ACCESS_HOST_VISIBLE = 0x1 << 2,
  NGFX_ACCESS_DEVICE_VISIBLE = 0x1 << 3,
  NGFX_ACCESS_HOST_COHERENT = 0x1 << 4,
  NGFX_ACCESS_HOST_CACHED = 0x1 << 5,
};

inline NGFXResourceAccessFlag operator|(NGFXResourceAccessFlag const& lhs,
  NGFXResourceAccessFlag const& rhs)
{
  return NGFXResourceAccessFlag(uint32(lhs) | uint32(rhs));
}

enum NGFXResourceCreationFlag
{
  NGFX_RESOURCE_DYNAMIC = 0,
  NGFX_RESOURCE_STATIC = 1,
  NGFX_RESOURCE_TRANSFER_SRC = 2,
  NGFX_RESOURCE_TRANSFER_DST = 4
};

inline NGFXResourceCreationFlag operator|(NGFXResourceCreationFlag const& lhs,
  NGFXResourceCreationFlag const& rhs)
{
  return NGFXResourceCreationFlag(uint32(lhs) | uint32(rhs));
}

enum NGFXResourceOrigin
{
  NGFX_RESOURCE_ORIGIN_USER,
  NGFX_RESOURCE_ORIGIN_SWAPCHAIN,
};

/**
* Same as VkImageAspectFlagBits
*/
enum NGFXTextureAspectFlag
{
  NGFX_ASPECT_COLOR = 1,
  NGFX_ASPECT_DEPTH = 1 << 1,
  NGFX_ASPECT_STENCIL = 1 << 2,
  NGFX_ASPECT_METADATA = 1 << 3,
};

inline NGFXTextureAspectFlag operator|(NGFXTextureAspectFlag const& lhs,
  NGFXTextureAspectFlag const& rhs)
{
  return NGFXTextureAspectFlag(uint32(lhs) | uint32(rhs));
}

enum NGFXShaderFormat
{
  NGFX_SHADER_FORMAT_TEXT,
  NGFX_SHADER_FORMAT_BYTE_CODE,
};

enum NGFXShaderLang
{
  NGFX_SHADER_LANG_GLSL,
  NGFX_SHADER_LANG_ESSL,
  NGFX_SHADER_LANG_VKGLSL,
  NGFX_SHADER_LANG_HLSL,
  NGFX_SHADER_LANG_METALSL,
};

enum NGFXShaderProfile
{
  NGFX_SHADER_PROFILE_LEGACY, // SM4_x, ES2.0,
  NGFX_SHADER_PROFILE_MODERN, // SM5_x, ES3.1, GLSL4XX, MSL
};

enum NGFXShaderSemantic
{
  NGFX_SEMANTIC_POSITION,
  NGFX_SEMANTIC_COLOR,
  NGFX_SEMANTIC_NORMAL,
};

enum NGFXShaderDataType
{
  NGFX_SHADER_VAR_BOOL,
  NGFX_SHADER_VAR_BOOL2,
  NGFX_SHADER_VAR_BOOL3,
  NGFX_SHADER_VAR_BOOL4,
  NGFX_SHADER_VAR_INT,
  NGFX_SHADER_VAR_INT2,
  NGFX_SHADER_VAR_INT3,
  NGFX_SHADER_VAR_INT4,
  NGFX_SHADER_VAR_UINT,
  NGFX_SHADER_VAR_UINT2,
  NGFX_SHADER_VAR_UINT3,
  NGFX_SHADER_VAR_UINT4,
  NGFX_SHADER_VAR_FLOAT,
  NGFX_SHADER_VAR_FLOAT2,
  NGFX_SHADER_VAR_FLOAT3,
  NGFX_SHADER_VAR_FLOAT4,
  NGFX_SHADER_VAR_MAT2,
  NGFX_SHADER_VAR_MAT2X3,
  NGFX_SHADER_VAR_MAT2X4,
  NGFX_SHADER_VAR_MAT3X2,
  NGFX_SHADER_VAR_MAT3,
  NGFX_SHADER_VAR_MAT3X4,
  NGFX_SHADER_VAR_MAT4X2,
  NGFX_SHADER_VAR_MAT4X3,
  NGFX_SHADER_VAR_MAT4,
  NGFX_SHADER_VAR_UNKNOWN
};

enum NGFXShaderBindType
{
  NGFX_SHADER_BIND_UNDEFINED = 0,
  NGFX_SHADER_BIND_BLOCK = 0x1,
  NGFX_SHADER_BIND_SAMPLER = 0x1 << 1,
  NGFX_SHADER_BIND_SAMPLED_IMAGE = 0x1 << 2,
  NGFX_SHADER_BIND_SAMPLER_IMAGE_COMBINE = (NGFX_SHADER_BIND_SAMPLER | NGFX_SHADER_BIND_SAMPLED_IMAGE),
  NGFX_SHADER_BIND_STORAGE_BUFFER = 0x1 << 3,  // StructedBuffer
  NGFX_SHADER_BIND_STORAGE_IMAGE = 0x1 << 4,   // RWTexture(SRV)?
  NGFX_SHADER_BIND_TEXEL_BUFFER = 0x1 << 5,
  NGFX_SHADER_BIND_RWTEXEL_BUFFER = 0x1 << 6,  // RWTexture(UAV)
  NGFX_SHADER_BIND_CONSTANTS = 0x1 << 7
};

#if __cplusplus

template <bool ThreadSafe>
struct NGFXRefCounted
{
  NGFXRefCounted() {}
  virtual ~NGFXRefCounted() {}

  int32 AddInternalRef()
  {
    return __k3d_intrinsics__::AtomicIncrement(&m_IntRef);
  }

  int32 ReleaseInternal()
  {
    auto c = __k3d_intrinsics__::AtomicDecrement(&m_IntRef);
    if (m_IntRef == 0)
    {
      delete this;
    }
    return c;
  }

  int32 Release()
  {
    auto c = __k3d_intrinsics__::AtomicDecrement(&m_ExtRef);
    if (m_ExtRef == 0)
    {
      ReleaseInternal();
    }
    return c;
  }

  int32 AddRef()
  {
    return __k3d_intrinsics__::AtomicIncrement(&m_ExtRef);
  }

private:
  int32 m_IntRef = 1;
  int32 m_ExtRef = 1;
};

/* Unthread-safe RefCounted*/
template <>
struct NGFXRefCounted<false>
{
  NGFXRefCounted() {}
  virtual ~NGFXRefCounted() {}

  int32 AddInternalRef()
  {
    m_IntRef++;
    return m_IntRef;
  }

  int32 ReleaseInternal()
  {
    m_IntRef--;
    int32 c = m_IntRef;
    if (m_IntRef == 0)
    {
      delete this;
    }
    return c;
  }

  int32 Release()
  {
    m_ExtRef--;
    int32 c = m_ExtRef;
    if (m_ExtRef == 0)
    {
      ReleaseInternal();
    }
    return c;
  }

  int32 AddRef()
  {
    m_ExtRef++;
    return m_ExtRef;
  }

private:
  int32 m_IntRef = 1;
  int32 m_ExtRef = 1;
};

template <bool ThreadSafe>
struct NGFXNamedObject : public NGFXRefCounted<ThreadSafe>
{
  virtual void SetName(const char *) {}

protected:
  void operator delete (void * Ptr)
  {
    __k3d_free__(Ptr, 0);
  }
};

template <class NGFXObj>
class NGFXPtr
{
public:
  explicit NGFXPtr(NGFXObj * pObj)
    : m_Ptr(pObj)
  {
  }

  NGFXPtr(NGFXPtr<NGFXObj> const& Other)
    : m_Ptr(Other.m_Ptr)
  {
    if(m_Ptr)
      m_Ptr->AddInternalRef();
  }

  NGFXPtr()
    : m_Ptr(nullptr)
  {
  }

  ~NGFXPtr()
  {
    if (m_Ptr)
    {
      m_Ptr->ReleaseInternal();
    }
    m_Ptr = nullptr;
  }

  NGFXObj& operator*() const { return *m_Ptr; }
  NGFXObj* operator->() const { return m_Ptr; }

  explicit operator bool() const
  {
    return m_Ptr != nullptr;
  }

  void Swap(NGFXPtr& Other)
  {
    NGFXObj * const pValue = Other.m_Ptr;
    Other.m_Ptr = m_Ptr;
    m_Ptr = pValue;
  }

  NGFXPtr& operator=(const NGFXPtr& Other)
  {
    typedef NGFXPtr<NGFXObj> ThisType;
    if (&Other != this)
    {
      ThisType(Other).Swap(*this);
    }
    return *this;
  }

  NGFXObj* Get() const
  {
    return m_Ptr;
  }

  NGFXObj** GetAddressOf() const
  {
    return &m_Ptr;
  }

private:
  NGFXObj* m_Ptr;
};

/**
* Format. Lang. Profile. Stage. Entry
*/
struct NGFXShaderDesc
{
  NGFXShaderFormat Format;
  NGFXShaderLang Lang;
  NGFXShaderProfile Profile;
  NGFXShaderType Stage;
  k3d::String EntryFunction;
};

struct NGFXShaderAttribute
{
  k3d::String VarName;
  NGFXShaderSemantic VarSemantic;
  NGFXShaderDataType VarType;
  uint32 VarLocation;
  uint32 VarBindingPoint;
  uint32 VarCount;

  NGFXShaderAttribute(const k3d::String& name,
    NGFXShaderSemantic semantic,
    NGFXShaderDataType dataType,
    uint32 location,
    uint32 bindingPoint,
    uint32 varCount)
    : VarName(name)
    , VarSemantic(semantic)
    , VarType(dataType)
    , VarLocation(location)
    , VarBindingPoint(bindingPoint)
    , VarCount(varCount)
  {
  }
};

struct NGFXShaderUniform
{
  NGFXShaderUniform() K3D_NOEXCEPT
    : VarType(NGFXShaderDataType::NGFX_SHADER_VAR_UNKNOWN)
    , VarName("")
    , VarOffset(0)
    , VarSzArray(0)
  {
  }

  NGFXShaderUniform(NGFXShaderDataType type,
    k3d::String const& name,
    uint32 offset,
    uint32 szArray = 0)
    : VarType(type)
    , VarName(name)
    , VarOffset(offset)
    , VarSzArray(szArray)
  {
  }

  virtual ~NGFXShaderUniform() {}

  NGFXShaderDataType VarType;
  k3d::String VarName;
  uint32 VarOffset;
  uint32 VarSzArray;
};

class NGFXShaderConstant : public NGFXShaderUniform
{
public:
  NGFXShaderConstant() {}
  ~NGFXShaderConstant() override {}
};

struct NGFXShaderBinding
{
  NGFXShaderBinding() K3D_NOEXCEPT
    : VarType(NGFXShaderBindType::NGFX_SHADER_BIND_UNDEFINED)
    , VarName("")
    , VarStage(NGFX_SHADER_TYPE_VERTEX)
    , VarNumber(0)
  {
  }

  NGFXShaderBinding(NGFXShaderBindType t, std::string n, NGFXShaderType st, uint32 num)
    : VarType(t)
    , VarName(n.c_str())
    , VarStage(st)
    , VarNumber(num)
  {
  }

  NGFXShaderBindType VarType;
  k3d::String VarName;
  NGFXShaderType VarStage;
  uint32 VarNumber;
};

struct NGFXShaderSet
{
  typedef uint32 VarIndex;
  /*bool operator==(NGFXShaderSet const &rhs)
  {
  return ...
  }*/
};

struct NGFXShaderBindingTable
{
  ::k3d::DynArray<NGFXShaderBinding> Bindings;
  ::k3d::DynArray<NGFXShaderUniform> Uniforms;
  ::k3d::DynArray<NGFXShaderSet::VarIndex> Sets;

  NGFXShaderBindingTable() K3D_NOEXCEPT = default;

  NGFXShaderBindingTable(NGFXShaderBindingTable const& rhs)
  {
    Bindings = rhs.Bindings;
    Uniforms = rhs.Uniforms;
    Sets = rhs.Sets;
  }

  NGFXShaderBindingTable& AddBinding(NGFXShaderBinding&& binding)
  {
    this->Bindings.Append(binding);
    return *this;
  }

  NGFXShaderBindingTable& AddUniform(NGFXShaderUniform&& uniform)
  {
    this->Uniforms.Append(uniform);
    return *this;
  }

  NGFXShaderBindingTable& AddSet(NGFXShaderSet::VarIndex const& set)
  {
    this->Sets.Append(set);
    return *this;
  }
};

typedef ::k3d::DynArray<NGFXShaderAttribute> NGFXShaderAttributes;
///
struct NGFXShaderBundle
{
  NGFXShaderDesc Desc;
  NGFXShaderBindingTable BindingTable;
  NGFXShaderAttributes Attributes;
  k3d::String RawData;
};
struct NGFXBlendState
{
  bool Enable;
  uint32 ColorWriteMask;

  NGFXBlendFactor Src;
  NGFXBlendFactor Dest;
  NGFXBlendOperation Op;

  NGFXBlendFactor SrcBlendAlpha;
  NGFXBlendFactor DestBlendAlpha;
  NGFXBlendOperation BlendAlphaOp;

  NGFXBlendState()
  {
    Enable = false;
    Src = NGFXBlendFactor::NGFX_BLEND_FACTOR_ONE;
    Dest = NGFXBlendFactor::NGFX_BLEND_FACTOR_ZERO;
    Op = NGFXBlendOperation::NGFX_BLEND_OP_ADD;

    SrcBlendAlpha = NGFXBlendFactor::NGFX_BLEND_FACTOR_ONE;
    DestBlendAlpha = NGFXBlendFactor::NGFX_BLEND_FACTOR_ZERO;
    BlendAlphaOp = NGFXBlendOperation::NGFX_BLEND_OP_ADD;
    ColorWriteMask = 0xf;
  }
};

struct NGFXAttachmentState
{
  NGFXBlendState Blend;
  NGFXPixelFormat Format; // for metal
};

struct NGFXRasterizerState
{
  NGFXFillMode FillMode;
  NGFXCullMode CullMode;
  bool FrontCCW;
  int32 DepthBias;
  float DepthBiasClamp;
  bool DepthClipEnable;
  bool MultiSampleEnable;
  NGFXMultiSampleFlag MSFlag;

  NGFXRasterizerState()
  {
    FillMode = NGFXFillMode::NGFX_FILL_MODE_SOLID;
    CullMode = NGFXCullMode::NGFX_CULL_MODE_BACK;
    FrontCCW = false;
    DepthBias = 0;
    DepthBiasClamp = 0.0f;
    DepthClipEnable = true;
    MultiSampleEnable = false;
  }
};

struct NGFXDepthStencilState
{
  struct Op
  {
    NGFXStencilOp StencilFailOp;
    NGFXStencilOp DepthStencilFailOp;
    NGFXStencilOp StencilPassOp;
    NGFXComparisonFunc StencilFunc;

    Op()
    {
      StencilFailOp = NGFXStencilOp::NGFX_STENCIL_OP_KEEP;
      DepthStencilFailOp = NGFXStencilOp::NGFX_STENCIL_OP_KEEP;
      StencilPassOp = NGFXStencilOp::NGFX_STENCIL_OP_KEEP;
      StencilFunc = NGFXComparisonFunc::NGFX_COMPARISON_FUNCTION_ALWAYS;
    }
  };

  bool DepthEnable;
  NGFXDepthWriteMask DepthWriteMask;
  NGFXComparisonFunc DepthFunc;

  bool StencilEnable;
  uint8 StencilReadMask;
  uint8 StencilWriteMask;
  Op FrontFace;
  Op BackFace;

  NGFXDepthStencilState()
  {
    DepthEnable = false;
    DepthWriteMask = NGFXDepthWriteMask::NGFX_DEPTH_WRITE_MASK_ALL;
    DepthFunc = NGFXComparisonFunc::NGFX_COMPARISON_FUNCTION_LESS;

    StencilEnable = false;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;
  }
};

struct NGFXSamplerState
{
  struct Filter
  {
    NGFXFilterMethod MinFilter;
    NGFXFilterMethod MagFilter;
    NGFXFilterMethod MipMapFilter;
    NGFXFilterReductionType ReductionType;
    Filter()
    {
      MinFilter = NGFXFilterMethod::NGFX_FILTER_METHOD_LINEAR;
      MagFilter = NGFXFilterMethod::NGFX_FILTER_METHOD_LINEAR;
      MipMapFilter = NGFXFilterMethod::NGFX_FILTER_METHOD_LINEAR;
      ReductionType = NGFXFilterReductionType::EFRT_Standard;
    }
  };

  Filter Filter;
  NGFXAddressMode U, V, W;
  float MipLODBias;
  uint32 MaxAnistropy;
  NGFXComparisonFunc ComparisonFunc;
  float BorderColor[4];
  float MinLOD;
  float MaxLOD;

  NGFXSamplerState()
    : U(NGFXAddressMode::NGFX_ADDRESS_MODE_WRAP)
    , V(NGFXAddressMode::NGFX_ADDRESS_MODE_WRAP)
    , W(NGFXAddressMode::NGFX_ADDRESS_MODE_WRAP)
    , MipLODBias(0)
    , MaxAnistropy(16)
    , ComparisonFunc(NGFXComparisonFunc::NGFX_COMPARISON_FUNCTION_LESS_EQUAL)
    , MinLOD(0)
    , MaxLOD(3.402823466e+38f)
  {
  }
};

struct NGFXVertexInputState
{
  enum
  {
    kInvalidValue = -1,
    kMaxVertexLayouts = 4,
    kMaxVertexBindings = 8,
  };

  struct Attribute
  {
    Attribute(NGFXVertexFormat const& format = NGFX_VERTEX_FORMAT_FLOAT3X32,
      uint32 offset = kInvalidValue,
      uint32 slot = kInvalidValue)
      : Format(format)
      , OffSet(offset)
      , Slot(slot)
    {
    }

    NGFXVertexFormat Format /* = NGFX_VERTEX_FORMAT_FLOAT3X32*/;
    uint32 OffSet /* = kInvalidValue*/;
    uint32 Slot /* = kInvalidValue*/;
  };

  struct Layout
  {
    Layout(NGFXVertexInputRate const& inputRate = NGFX_VERTEX_INPUT_RATE_PER_VERTEX,
      uint32 stride = kInvalidValue)
      : Rate(inputRate)
      , Stride(stride)
    {
    }

    NGFXVertexInputRate Rate /* = NGFX_VERTEX_INPUT_RATE_PER_VERTEX */;
    uint32 Stride /* = kInvalidValue*/;
  };

  NGFXVertexInputState() {}

  Attribute Attribs[kMaxVertexBindings];
  Layout Layouts[kMaxVertexLayouts];
};

using AttachmentStateArray = k3d::DynArray<NGFXAttachmentState>;

struct NGFXRenderPipelineDesc
{
  NGFXRenderPipelineDesc()
    : PrimitiveTopology(NGFX_PRIMITIVE_TRIANGLES)
    , PatchControlPoints(0)
    , DepthAttachmentFormat()
  {
  }
  NGFXRasterizerState Rasterizer;
  AttachmentStateArray AttachmentsBlend;  // 
  NGFXDepthStencilState DepthStencil;
  NGFXPixelFormat DepthAttachmentFormat;     // for metal
  NGFXVertexInputState InputState;
  // InputAssemblyState
  NGFXPrimitiveType PrimitiveTopology /* = rhi::NGFX_PRIMITIVE_TRIANGLES */;
  // Tessellation Patch
  uint32 PatchControlPoints /* = 0*/;

  NGFXShaderBundle VertexShader;
  NGFXShaderBundle PixelShader;
  NGFXShaderBundle GeometryShader;
  NGFXShaderBundle DomainShader;
  NGFXShaderBundle HullShader;
};

struct NGFXRect
{
  long Left;
  long Top;
  long Right;
  long Bottom;
};

struct NGFXViewportDesc
{
  NGFXViewportDesc()
    : NGFXViewportDesc(0.f, 0.f)
  {
  }

  NGFXViewportDesc(float width,
    float height,
    float left = 0.0f,
    float top = 0.0f,
    float minDepth = 0.0f,
    float maxDepth = 1.0f)
    : Left(left)
    , Top(top)
    , Width(width)
    , Height(height)
    , MinDepth(minDepth)
    , MaxDepth(maxDepth)
  {
  }

  float Left;
  float Top;
  float Width;
  float Height;
  float MinDepth;
  float MaxDepth;
};

struct NGFXVertexBufferView
{
  uint64 BufferLocation;
  uint32 SizeInBytes;
  uint32 StrideInBytes;
};

struct NGFXIndexBufferView
{
  uint64 BufferLocation;
  uint32 SizeInBytes;
};

struct NGFXSwapChainDesc
{
  NGFXPixelFormat Format;
  uint32 Width;
  uint32 Height;
  uint32 NumBuffers;
};

/**
* Format, Width, Height, Depth, MipLevel, Layers
*/
struct NGFXTextureDesc
{
  NGFXPixelFormat Format;
  uint32 Width;
  uint32 Height;
  uint32 Depth;
  uint32 MipLevels;
  uint32 Layers;

  NGFXTextureDesc(
    NGFXPixelFormat _Format = NGFX_PIXEL_FORMAT_RGBA8_UNORM
    , uint32 _Width = 0
    , uint32 _Height = 0
    , uint32 _Depth = 1
    , uint32 _MipLevels = 1
    , uint32 _Layers = 1)
    : Format(_Format)
    , Width(_Width)
    , Height(_Height)
    , Depth(_Depth)
    , MipLevels(_MipLevels)
    , Layers(_Layers)
  {
  }

  bool IsTex1D() const { return Depth == 1 && Layers == 1 && Height == 1; }
  bool IsTex2D() const { return Depth == 1 && Layers == 1 && Height > 1; }
  bool IsTex3D() const { return Depth > 1 && Layers == 1; }
};

/* in d3d12 and vulkan, each resource has a memory for resource foot print
* which needs extra memory. */
struct NGFXResourceDesc
{
  NGFXResourceType Type;
  NGFXResourceAccessFlag Flag;
  NGFXResourceCreationFlag CreationFlag;
  NGFXResourceViewTypeBits ViewFlags; // for resource view prediction
  NGFXResourceOrigin Origin;
  union
  {
    NGFXTextureDesc TextureDesc;
    uint64 Size;
  };
  NGFXResourceDesc()
    : Type(NGFX_BUFFER)
    , Flag(NGFX_ACCESS_READ_AND_WRITE)
    , CreationFlag(NGFX_RESOURCE_DYNAMIC)
    , ViewFlags(NGFX_RESOURCE_SHADER_RESOURCE_VIEW)
    , Origin(NGFX_RESOURCE_ORIGIN_USER)
  {
  }
  NGFXResourceDesc(NGFXResourceType t,
    NGFXResourceAccessFlag f,
    NGFXResourceCreationFlag cf,
    NGFXResourceViewTypeBits viewType)
    : Type(t)
    , Flag(f)
    , CreationFlag(cf)
    , ViewFlags(viewType)
    , Origin(NGFX_RESOURCE_ORIGIN_USER)
  {
  }
};

/*same as VkImageSubresource */
struct NGFXTextureSpec
{
  NGFXTextureAspectFlag Aspect;
  uint32 MipLevel;
  uint32 ArrayLayer;
};

struct NGFXBufferSpec
{
  uint64 FirstElement; // for vulkan, offset = FirstElement * StructureByteStride;
  uint64 NumElements; // range = NumElements * StructureByteStride
  uint64 StructureByteStride;
};

/**
* Format
* BufferSpec(First, NumElement, Stride)
* TextureSpec
*/
struct NGFXUAVDesc
{
  NGFXViewDimension Dim;
  NGFXPixelFormat Format;
  union
  {
    NGFXBufferSpec Buffer;
    NGFXTextureSpec Texture;
  };
};

/**
* ReadOnly
*/
struct NGFXSRVDesc
{
  NGFXViewDimension Dim;
  NGFXPixelFormat Format;
  union
  {
    NGFXBufferSpec Buffer;
    NGFXTextureSpec Texture;
  };
};

struct NGFXResourceViewDesc
{
  NGFXResourceViewTypeBits ViewType;
  union
  {
    NGFXTextureSpec TextureSpec;
  };
};
/**
* Same as VkSubresourceLayout
*/
struct NGFXSubResourceLayout
{
  uint64 Offset = 0;
  uint64 Size = 0;
  uint64 RowPitch = 0;
  uint64 ArrayPitch = 0;
  uint64 DepthPitch = 0;
};

struct NGFXCopyBufferRegion
{
  uint64 SrcOffSet;
  uint64 DestOffSet;
  uint64 CopySize;
};

/* see also VkBufferImageCopy(vk) and
* D3D12_PLACED_SUBRESOURCE_FOOTPRINT(d3d12)
*/
struct NGFXSubResourceFootprint
{
  NGFXTextureDesc Dimension;
  NGFXSubResourceLayout SubLayout;
};

struct NGFXPlacedSubResourceFootprint
{
  uint32 BufferOffSet = 0; // VkBufferImageCopy#bufferOffset
  int32 TOffSetX = 0, TOffSetY = 0,
    TOffSetZ = 0;                  // VkBufferImageCopy#imageOffset
  NGFXSubResourceFootprint Footprint = {}; // VkBufferImageCopy#imageExtent
};

/**
* IndexCountPerInstance
* InstanceCount
* StartLocation
* BaseVertexLocation
* StartInstanceLocation
*/
struct NGFXDrawIndexedInstancedParam
{
  NGFXDrawIndexedInstancedParam(uint32 indexCountPerInst,
    uint32 instCount,
    uint32 startLoc = 0,
    uint32 baseVerLoc = 0,
    uint32 sInstLoc = 1)
    : IndexCountPerInstance(indexCountPerInst)
    , InstanceCount(instCount)
    , StartIndexLocation(startLoc)
    , BaseVertexLocation(baseVerLoc)
    , StartInstanceLocation(sInstLoc)
  {
  }
  uint32 IndexCountPerInstance;
  uint32 InstanceCount;
  uint32 StartIndexLocation;
  uint32 BaseVertexLocation;
  uint32 StartInstanceLocation;
};

struct NGFXDrawInstancedParam
{
  NGFXDrawInstancedParam(uint32 vertexPerInst,
    uint32 instances,
    uint32 startLocation = 0,
    uint32 startInstLocation = 0)
    : VertexCountPerInstance(vertexPerInst)
    , InstanceCount(instances)
    , StartVertexLocation(startLocation)
    , StartInstanceLocation(startInstLocation)
  {
  }
  uint32 VertexCountPerInstance;
  uint32 InstanceCount;
  uint32 StartVertexLocation;
  uint32 StartInstanceLocation;
};
#if 0
typedef SharedPtr<struct NGFXCommandQueue> NGFXCommandQueueRef;
typedef SharedPtr<struct NGFXCommandBuffer> NGFXCommandBufferRef;
typedef SharedPtr<struct NGFXRenderCommandEncoder> NGFXRenderCommandEncoderRef;
typedef SharedPtr<struct NGFXComputeCommandEncoder> NGFXComputeCommandEncoderRef;
typedef SharedPtr<struct NGFXParallelRenderCommandEncoder> NGFXParallelRenderCommandEncoderRef;
typedef SharedPtr<struct NGFXDevice> NGFXDeviceRef;
typedef SharedPtr<struct NGFXFactory> NGFXFactoryRef;
typedef SharedPtr<struct NGFXResource> NGFXResourceRef;
typedef SharedPtr<struct NGFXTexture> NGFXTextureRef;
typedef SharedPtr<struct NGFXBuffer> NGFXBufferRef;
typedef SharedPtr<struct NGFXTextureView> TextureViewRef;
typedef SharedPtr<struct NGFXBufferView> BufferViewRef;
typedef SharedPtr<struct NGFXShaderResourceView> NGFXSRVRef;
typedef SharedPtr<struct NGFXUnorderedAccessView> NGFXUAVRef;
typedef SharedPtr<struct NGFXPipelineState> NGFXPipelineStateRef;
typedef SharedPtr<struct NGFXRenderPipelineState> RenderPipelineStateRef;
typedef SharedPtr<struct NGFXComputePipelineState> ComputePipelineStateRef;
typedef SharedPtr<struct NGFXRenderPipelineEncoder> RenderPipelineEncoderRef;
typedef SharedPtr<struct NGFXComputePipelineEncoder> ComputePipelineEncoderRef;
typedef SharedPtr<struct NGFXSwapChain> NGFXSwapChainRef;
typedef SharedPtr<struct NGFXSampler> NGFXSamplerRef;
typedef SharedPtr<struct NGFXBindingGroup> NGFXBindingGroupRef;
typedef SharedPtr<struct NGFXRenderTarget> NGFXRenderTargetRef;
typedef SharedPtr<struct NGFXRenderpass> NGFXRenderpassRef;
typedef SharedPtr<struct NGFXFence> NGFXFenceRef;
typedef SharedPtr<struct NGFXPipelineLayout> NGFXPipelineLayoutRef;
#endif

typedef struct NGFXCommandQueue * NGFXCommandQueueRef;
typedef struct NGFXCommandBuffer * NGFXCommandBufferRef;
typedef struct NGFXRenderCommandEncoder * NGFXRenderCommandEncoderRef;
typedef struct NGFXComputeCommandEncoder * NGFXComputeCommandEncoderRef;
typedef struct NGFXParallelRenderCommandEncoder * NGFXParallelRenderCommandEncoderRef;
typedef struct NGFXDevice * NGFXDeviceRef;
typedef struct NGFXFactory * NGFXFactoryRef;
typedef struct NGFXResource * NGFXResourceRef;
typedef struct NGFXTexture * NGFXTextureRef;
typedef struct NGFXBuffer * NGFXBufferRef;
typedef struct NGFXTextureView * TextureViewRef;
typedef struct NGFXBufferView * BufferViewRef;

// buffer: texel buffer ReadOnly 
// texture: sampled ReadOnly
typedef struct NGFXShaderResourceView * NGFXSRVRef;

// buffer: RWBuffer, RWStructedBuffer,
// texture: RWTexture
typedef struct NGFXUnorderedAccessView * NGFXUAVRef;
typedef struct NGFXPipeline * NGFXPipelineStateRef;
typedef struct NGFXRenderPipeline * RenderPipelineStateRef;
typedef struct NGFXComputePipeline * ComputePipelineStateRef;
typedef struct NGFXRenderPipelineEncoder * RenderPipelineEncoderRef;
typedef struct NGFXComputePipelineEncoder * ComputePipelineEncoderRef;
typedef struct NGFXSwapChain * NGFXSwapChainRef;
typedef struct NGFXSampler * NGFXSamplerRef;
typedef struct NGFXBindingGroup * NGFXBindingGroupRef;
typedef struct NGFXRenderTarget * NGFXRenderTargetRef;
typedef struct NGFXRenderpass * NGFXRenderpassRef;
/**
* Semaphore used for GPU to GPU syncs,
* Specifically used to sync queue submissions (on the same or different
* queues), Set on GPU, Wait on GPU (inter-queue) Events: Set anywhere, Wait on
* GPU (intra-queue)
*/
struct NGFXSemaphore;

/**
* @see
* https://www.reddit.com/r/vulkan/comments/47tc3s/differences_between_vkfence_vkevent_and/?
* Fence is GPU to CPU syncs
* Set on GPU, Wait on CPU
*/
typedef struct NGFXFence * NGFXFenceRef;

/**
* Vulkan has pipeline layout,
* D3D12 is root signature.
* PipelineLayout defines shaders impose constraints on table layout.
*/
typedef struct NGFXPipelineLayout * NGFXPipelineLayoutRef;

struct NGFXResource : public NGFXNamedObject<false>
{
  //  virtual ~NGFXResource() {}
  virtual void* Map(uint64 start, uint64 size) = 0;
  virtual void UnMap() = 0;

  virtual uint64 GetLocation() const { return 0; }
  virtual NGFXResourceDesc GetDesc() const = 0;

  /**
  * Vulkan: texture uses image layout as resource state <br/>
  * D3D12: used for transition, maybe used as ShaderVisiblity determination in <br/>
  * STATIC_SAMPLER and descriptor table
  */
  virtual NGFXResourceState GetState() const { return NGFX_RESOURCE_STATE_UNKNOWN; }
  virtual uint64 GetSize() const = 0;
};

struct NGFXSampler : public NGFXNamedObject<false>
{
  virtual NGFXSamplerState GetSamplerDesc() const = 0;
};

struct NGFXShaderResourceView : public NGFXNamedObject<false>
{
  virtual NGFXResourceRef GetResource() const = 0;
  virtual NGFXSRVDesc GetDesc() const = 0;
};

struct NGFXTexture : public NGFXResource
{
  virtual NGFXSamplerRef GetSampler() const = 0;
  virtual void BindSampler(NGFXSamplerRef) = 0;
  virtual void SetResourceView(NGFXSRVRef) = 0;
  virtual NGFXSRVRef GetResourceView() const = 0;
};

struct NGFXBuffer : public NGFXResource
{
};

struct NGFXUnorderedAccessView : public NGFXNamedObject<false>
{
};

struct NGFXBindingGroup : public NGFXNamedObject<true>
{
  virtual void Update(uint32 bindSet, NGFXResourceRef) = 0;
  virtual void Update(uint32 bindSet, NGFXUAVRef) = 0;
  virtual void Update(uint32 bindSet, NGFXSamplerRef) {};
  virtual uint32 GetSlotNum() const { return 0; }
};

/**
* Vulkan has pipeline layout,
* D3D12 is root signature.
* PipelineLayout defines shaders impose constraints on table layout.
*/
struct NGFXPipelineLayout : public NGFXNamedObject<true>
{
  virtual NGFXBindingGroupRef ObtainBindingGroup() = 0;
};

/**
* @see
* https://www.reddit.com/r/vulkan/comments/47tc3s/differences_between_vkfence_vkevent_and/?
* Fence is GPU to CPU syncs
* Set on GPU, Wait on CPU
*/
struct NGFXFence : public NGFXNamedObject<true>
{
  virtual void Signal(int32 fenceVal) = 0;
  virtual void Reset() {}
  virtual void WaitFor(uint64 time) = 0;
};

struct NGFXRenderTarget
{
  virtual ~NGFXRenderTarget() {}
  virtual void SetClearColor(kMath::Vec4f clrColor) = 0;
  virtual void SetClearDepthStencil(float depth, uint32 stencil) = 0;
  virtual NGFXResourceRef GetBackBuffer() = 0;
};

struct NGFXComputePipelineDesc
{
  NGFXShaderBundle ComputeShader;
};

struct NGFXPipeline : public NGFXNamedObject<true>
{
  virtual NGFXPipelineType GetType() const = 0;

  // rebuild pipeline state if is dirty
  virtual void Rebuild() = 0;

  // storage options
  virtual void SavePSO(::k3d::String const& Path) {}
  virtual void LoadPSO(::k3d::String const& Path) {}
};

struct NGFXRenderPipeline : public NGFXPipeline
{
  virtual void SetRasterizerState(const NGFXRasterizerState&) = 0;
  virtual void SetBlendState(const NGFXBlendState&) = 0;
  virtual void SetDepthStencilState(const NGFXDepthStencilState&) = 0;
  virtual void SetPrimitiveTopology(const NGFXPrimitiveType) = 0;
  virtual void SetSampler(NGFXSamplerRef) = 0;
};

struct NGFXComputePipeline : public NGFXPipeline
{
};

using NGFXPipelineLayoutDesc = NGFXShaderBindingTable;

struct NGFXAttachmentDesc
{
  NGFXAttachmentDesc()
    : LoadAction(NGFX_LOAD_ACTION_CLEAR)
    , StoreAction(NGFX_STORE_ACTION_STORE)
  {
  }
  NGFXLoadAction LoadAction;
  NGFXStoreAction StoreAction;
  NGFXTextureRef pTexture;
};

struct NGFXColorAttachmentDesc : public NGFXAttachmentDesc
{
  kMath::Vec4f ClearColor;
};

using NGFXColorAttachmentArray = k3d::DynArray<NGFXColorAttachmentDesc>;

struct NGFXDepthAttachmentDesc : public NGFXAttachmentDesc
{
  float ClearDepth;
};

struct NGFXStencilAttachmentDesc : public NGFXAttachmentDesc
{
  float ClearStencil;
};

struct NGFXRenderPassDesc
{
  NGFXColorAttachmentArray ColorAttachments;
  k3d::SharedPtr<NGFXDepthAttachmentDesc> pDepthAttachment;
  k3d::SharedPtr<NGFXStencilAttachmentDesc> pStencilAttachment;
};

struct AttachmentDescriptor : public NGFXRefCounted<false>
{

};

struct RenderPassDescriptor : public NGFXRefCounted<false>
{

};

struct NGFXRenderpass : public NGFXNamedObject<false>
{
  virtual NGFXRenderPassDesc GetDesc() const = 0;
};

struct NGFXDevice : public NGFXNamedObject<false>
{
  enum Result
  {
    DeviceNotFound,
    DeviceFound
  };

  virtual NGFXResourceRef CreateResource(NGFXResourceDesc const&) = 0;
  virtual NGFXSRVRef CreateShaderResourceView(
    NGFXResourceRef,
    NGFXSRVDesc const&) = 0;

  virtual NGFXUAVRef CreateUnorderedAccessView(const NGFXResourceRef&, NGFXUAVDesc const&) = 0;

  virtual NGFXSamplerRef CreateSampler(const NGFXSamplerState&) = 0;

  virtual NGFXPipelineLayoutRef CreatePipelineLayout(
    NGFXPipelineLayoutDesc const& table) = 0;
  virtual NGFXFenceRef CreateFence() = 0;

  //  virtual NGFXRenderTargetRef NewRenderTarget(RenderTargetLayout const&) = 0;

  virtual NGFXRenderpassRef CreateRenderPass(NGFXRenderPassDesc const&) = 0;

  virtual NGFXPipelineStateRef CreateRenderPipelineState(
    NGFXRenderPipelineDesc const&,
    NGFXPipelineLayoutRef,
    NGFXRenderpassRef) = 0;

  virtual NGFXPipelineStateRef CreateComputePipelineState(
    NGFXComputePipelineDesc const&,
    NGFXPipelineLayoutRef) = 0;

  virtual NGFXCommandQueueRef CreateCommandQueue(NGFXCommandType const&) = 0;

  virtual void WaitIdle() {}

  /**
  * equal with d3d12's getcopyfootprint or vulkan's getImagesubreslayout.
  */
  virtual void QueryTextureSubResourceLayout(
    NGFXTextureRef,
    NGFXTextureSpec const& spec,
    NGFXSubResourceLayout*)
  {}
};

struct NGFXFactory : public NGFXNamedObject<false>
{
  virtual void EnumDevices(k3d::DynArray<NGFXDeviceRef>& Devices) = 0;
  virtual NGFXSwapChainRef CreateSwapchain(NGFXCommandQueueRef pCommandQueue,
    void* nPtr,
    NGFXSwapChainDesc&) = 0;
};

struct NGFXSwapChain : public NGFXNamedObject<false>
{
  virtual void Resize(uint32 Width, uint32 Height) = 0;
  virtual void Present() = 0;
  virtual NGFXTextureRef GetCurrentTexture() = 0;
};

struct NGFXTextureCopyLocation
{
  typedef ::k3d::DynArray<uint32> ResIds;
  typedef ::k3d::DynArray<NGFXPlacedSubResourceFootprint> ResFootprints;

  enum ESubResource
  {
    ESubResourceIndex,
    ESubResourceFootPrints
  };

  NGFXTextureCopyLocation(NGFXResourceRef ptrResource, ResIds subResourceIndex)
    : pResource(ptrResource)
    , SubResourceIndexes(subResourceIndex)
  {
  }

  NGFXTextureCopyLocation(NGFXResourceRef ptrResource, ResFootprints footprints)
    : pResource(ptrResource)
    , SubResourceFootPrints(footprints)
  {
  }

  NGFXTextureCopyLocation() = default;

  ~NGFXTextureCopyLocation() {}

  NGFXResourceRef pResource;
  ESubResource SubResourceType = ESubResourceIndex;
  ResIds SubResourceIndexes;
  ResFootprints SubResourceFootPrints;
};

struct NGFXCommandQueue : public NGFXNamedObject<true>
{
  virtual NGFXCommandBufferRef ObtainCommandBuffer(NGFXCommandReuseType const&) = 0;
};

struct NGFXCommandBuffer : public NGFXNamedObject<true>
{
  virtual void Commit(NGFXFenceRef pFence = nullptr) = 0;
  // For Vulkan, This command will be appended to the tail
  virtual void Present(NGFXSwapChainRef pSwapChain, NGFXFenceRef pFence) = 0;
  virtual void Reset() = 0;
  virtual NGFXRenderCommandEncoderRef RenderCommandEncoder(
    NGFXRenderPassDesc const&) = 0;
  virtual NGFXComputeCommandEncoderRef ComputeCommandEncoder() = 0;
  virtual NGFXParallelRenderCommandEncoderRef ParallelRenderCommandEncoder(
    NGFXRenderPassDesc const&) = 0;
  // blit
  virtual void CopyTexture(const NGFXTextureCopyLocation& Dest,
    const NGFXTextureCopyLocation& Src) = 0;
  //
  virtual void CopyBuffer(NGFXResourceRef Dest,
    NGFXResourceRef Src,
    NGFXCopyBufferRegion const& Region) = 0;
  virtual void Transition(NGFXResourceRef pResource,
    NGFXResourceState const&
    State /*, rhi::EPipelineStage const& Stage*/) = 0;
};

struct NGFXCommandEncoder : public NGFXNamedObject<true>
{
  virtual void SetPipelineState(uint32 HashCode, NGFXPipelineStateRef const&) = 0;
  virtual void SetBindingGroup(NGFXBindingGroupRef const&) = 0;
  virtual void EndEncode() = 0;
};

struct NGFXRenderCommandEncoder : public NGFXCommandEncoder
{
  virtual void SetScissorRect(const NGFXRect&) = 0;
  virtual void SetViewport(const NGFXViewportDesc&) = 0;
  virtual void SetIndexBuffer(const NGFXIndexBufferView& IBView) = 0;
  virtual void SetVertexBuffer(uint32 Slot, const NGFXVertexBufferView& VBView) = 0;
  virtual void SetPrimitiveType(NGFXPrimitiveType) = 0;
  virtual void DrawInstanced(NGFXDrawInstancedParam) = 0;
  virtual void DrawIndexedInstanced(NGFXDrawIndexedInstancedParam) = 0;
};

struct NGFXComputeCommandEncoder : public NGFXCommandEncoder
{
  virtual void Dispatch(uint32 GroupCountX,
    uint32 GroupCountY,
    uint32 GroupCountZ) = 0;
};
// Begin with a single rendering pass, encoded from multiple threads
// simultaneously.
struct NGFXParallelRenderCommandEncoder : public NGFXCommandEncoder
{
  virtual NGFXRenderCommandEncoderRef SubRenderCommandEncoder() = 0;
};

#endif // end Cpp

#endif