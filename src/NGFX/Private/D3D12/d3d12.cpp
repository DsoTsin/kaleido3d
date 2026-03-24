/**
 * NGFX Direct3D 12 Backend
 *
 * Implements the full ngfx interface on top of the D3D12 API.
 * Linked as a shared library (ngfx_dx12.dll) and exposes CreateFactory().
 */
#include <ngfx.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#undef near
#undef far

#include <string>
#include <vector>
#include <cassert>
#include <cstring>

using Microsoft::WRL::ComPtr;

#if _MSC_VER
#define NGFX_EXPORT __declspec(dllexport)
#else
#define NGFX_EXPORT __attribute__((visibility("default")))
#endif

typedef void(*ngfx_LogCallback)(int level, const char* msg);

// ---------------------------------------------------------------------------
// Format conversion
// ---------------------------------------------------------------------------
namespace d3d12 {

static DXGI_FORMAT pixelFormatToDXGI(ngfx::PixelFormat fmt) {
    switch (fmt) {
    case ngfx::PixelFormat::R8Unorm:             return DXGI_FORMAT_R8_UNORM;
    case ngfx::PixelFormat::R8Snorm:             return DXGI_FORMAT_R8_SNORM;
    case ngfx::PixelFormat::R8Uint:              return DXGI_FORMAT_R8_UINT;
    case ngfx::PixelFormat::R8Sint:              return DXGI_FORMAT_R8_SINT;
    case ngfx::PixelFormat::R16Unorm:            return DXGI_FORMAT_R16_UNORM;
    case ngfx::PixelFormat::R16Snorm:            return DXGI_FORMAT_R16_SNORM;
    case ngfx::PixelFormat::R16Uint:             return DXGI_FORMAT_R16_UINT;
    case ngfx::PixelFormat::R16Sint:             return DXGI_FORMAT_R16_SINT;
    case ngfx::PixelFormat::R16Float:            return DXGI_FORMAT_R16_FLOAT;
    case ngfx::PixelFormat::RG8Unorm:            return DXGI_FORMAT_R8G8_UNORM;
    case ngfx::PixelFormat::RG8Snorm:            return DXGI_FORMAT_R8G8_SNORM;
    case ngfx::PixelFormat::RG8Uint:             return DXGI_FORMAT_R8G8_UINT;
    case ngfx::PixelFormat::RG8Sint:             return DXGI_FORMAT_R8G8_SINT;
    case ngfx::PixelFormat::R32Uint:             return DXGI_FORMAT_R32_UINT;
    case ngfx::PixelFormat::R32Sint:             return DXGI_FORMAT_R32_SINT;
    case ngfx::PixelFormat::R32Float:            return DXGI_FORMAT_R32_FLOAT;
    case ngfx::PixelFormat::RG16Unorm:           return DXGI_FORMAT_R16G16_UNORM;
    case ngfx::PixelFormat::RG16Snorm:           return DXGI_FORMAT_R16G16_SNORM;
    case ngfx::PixelFormat::RG16Uint:            return DXGI_FORMAT_R16G16_UINT;
    case ngfx::PixelFormat::RG16Sint:            return DXGI_FORMAT_R16G16_SINT;
    case ngfx::PixelFormat::RG16Float:           return DXGI_FORMAT_R16G16_FLOAT;
    case ngfx::PixelFormat::RGBA8Unorm:          return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ngfx::PixelFormat::RGBA8Unorm_sRGB:     return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case ngfx::PixelFormat::RGBA8Snorm:          return DXGI_FORMAT_R8G8B8A8_SNORM;
    case ngfx::PixelFormat::RGBA8Uint:           return DXGI_FORMAT_R8G8B8A8_UINT;
    case ngfx::PixelFormat::RGBA8Sint:           return DXGI_FORMAT_R8G8B8A8_SINT;
    case ngfx::PixelFormat::BGRA8Unorm:          return DXGI_FORMAT_B8G8R8A8_UNORM;
    case ngfx::PixelFormat::BGRA8Unorm_sRGB:     return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    case ngfx::PixelFormat::RG11B10Float:        return DXGI_FORMAT_R11G11B10_FLOAT;
    case ngfx::PixelFormat::RGB9E5Float:         return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
    case ngfx::PixelFormat::RG32Uint:            return DXGI_FORMAT_R32G32_UINT;
    case ngfx::PixelFormat::RG32Sint:            return DXGI_FORMAT_R32G32_SINT;
    case ngfx::PixelFormat::RG32Float:           return DXGI_FORMAT_R32G32_FLOAT;
    case ngfx::PixelFormat::RGBA16Unorm:         return DXGI_FORMAT_R16G16B16A16_UNORM;
    case ngfx::PixelFormat::RGBA16Snorm:         return DXGI_FORMAT_R16G16B16A16_SNORM;
    case ngfx::PixelFormat::RGBA16Uint:          return DXGI_FORMAT_R16G16B16A16_UINT;
    case ngfx::PixelFormat::RGBA16Sint:          return DXGI_FORMAT_R16G16B16A16_SINT;
    case ngfx::PixelFormat::RGBA16Float:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case ngfx::PixelFormat::RGBA32Uint:          return DXGI_FORMAT_R32G32B32A32_UINT;
    case ngfx::PixelFormat::RGBA32Sint:          return DXGI_FORMAT_R32G32B32A32_SINT;
    case ngfx::PixelFormat::RGBA32Float:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case ngfx::PixelFormat::Depth16Unorm:        return DXGI_FORMAT_D16_UNORM;
    case ngfx::PixelFormat::Depth32Float:        return DXGI_FORMAT_D32_FLOAT;
    case ngfx::PixelFormat::Depth24UnormStencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case ngfx::PixelFormat::Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    default:                                     return DXGI_FORMAT_UNKNOWN;
    }
}

static ngfx::PixelFormat dxgiToPixelFormat(DXGI_FORMAT fmt) {
    switch (fmt) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:      return ngfx::PixelFormat::RGBA8Unorm;
    case DXGI_FORMAT_B8G8R8A8_UNORM:      return ngfx::PixelFormat::BGRA8Unorm;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:   return ngfx::PixelFormat::RGBA16Float;
    case DXGI_FORMAT_R11G11B10_FLOAT:      return ngfx::PixelFormat::RG11B10Float;
    case DXGI_FORMAT_D32_FLOAT:            return ngfx::PixelFormat::Depth32Float;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:    return ngfx::PixelFormat::Depth24UnormStencil8;
    default:                               return ngfx::PixelFormat::Invalid;
    }
}

static DXGI_FORMAT vertexFormatToDXGI(ngfx::PixelFormat fmt) {
    return pixelFormatToDXGI(fmt);
}

static D3D12_COMPARISON_FUNC comparisonToDX(ngfx::ComparisonFunction fn) {
    switch (fn) {
    case ngfx::ComparisonFunction::Never:        return D3D12_COMPARISON_FUNC_NEVER;
    case ngfx::ComparisonFunction::Less:         return D3D12_COMPARISON_FUNC_LESS;
    case ngfx::ComparisonFunction::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
    case ngfx::ComparisonFunction::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case ngfx::ComparisonFunction::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
    case ngfx::ComparisonFunction::NotEqual:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case ngfx::ComparisonFunction::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case ngfx::ComparisonFunction::Always:       return D3D12_COMPARISON_FUNC_ALWAYS;
    default:                                     return D3D12_COMPARISON_FUNC_NEVER;
    }
}

static D3D12_STENCIL_OP stencilOpToDX(ngfx::StencilOperation op) {
    switch (op) {
    case ngfx::StencilOperation::Keep:           return D3D12_STENCIL_OP_KEEP;
    case ngfx::StencilOperation::Zero:           return D3D12_STENCIL_OP_ZERO;
    case ngfx::StencilOperation::Replace:        return D3D12_STENCIL_OP_REPLACE;
    case ngfx::StencilOperation::IncrementClamp: return D3D12_STENCIL_OP_INCR_SAT;
    case ngfx::StencilOperation::DecrementClamp: return D3D12_STENCIL_OP_DECR_SAT;
    case ngfx::StencilOperation::Invert:         return D3D12_STENCIL_OP_INVERT;
    case ngfx::StencilOperation::IncrementWrap:  return D3D12_STENCIL_OP_INCR;
    case ngfx::StencilOperation::DecrementWrap:  return D3D12_STENCIL_OP_DECR;
    default:                                     return D3D12_STENCIL_OP_KEEP;
    }
}

static D3D12_BLEND blendFactorToDX(ngfx::BlendFactor f) {
    switch (f) {
    case ngfx::BlendFactor::Zero:              return D3D12_BLEND_ZERO;
    case ngfx::BlendFactor::One:               return D3D12_BLEND_ONE;
    case ngfx::BlendFactor::SrcColor:          return D3D12_BLEND_SRC_COLOR;
    case ngfx::BlendFactor::OneMinusSrcColor:  return D3D12_BLEND_INV_SRC_COLOR;
    case ngfx::BlendFactor::SrcAlpha:          return D3D12_BLEND_SRC_ALPHA;
    case ngfx::BlendFactor::OneMinusSrcAlpha:  return D3D12_BLEND_INV_SRC_ALPHA;
    case ngfx::BlendFactor::DestColor:         return D3D12_BLEND_DEST_COLOR;
    case ngfx::BlendFactor::OneMinusDestColor: return D3D12_BLEND_INV_DEST_COLOR;
    case ngfx::BlendFactor::DestAlpha:         return D3D12_BLEND_DEST_ALPHA;
    case ngfx::BlendFactor::OneMinusDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
    default:                                   return D3D12_BLEND_ZERO;
    }
}

static D3D12_BLEND_OP blendOpToDX(ngfx::BlendOperation op) {
    switch (op) {
    case ngfx::BlendOperation::Add:    return D3D12_BLEND_OP_ADD;
    case ngfx::BlendOperation::Sub:    return D3D12_BLEND_OP_SUBTRACT;
    case ngfx::BlendOperation::RevSub: return D3D12_BLEND_OP_REV_SUBTRACT;
    case ngfx::BlendOperation::Min:    return D3D12_BLEND_OP_MIN;
    case ngfx::BlendOperation::Max:    return D3D12_BLEND_OP_MAX;
    default:                           return D3D12_BLEND_OP_ADD;
    }
}

static D3D12_FILL_MODE fillModeToDX(ngfx::FillMode m) {
    return m == ngfx::FillMode::Line ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
}

static D3D12_CULL_MODE cullModeToDX(ngfx::CullMode m) {
    switch (m) {
    case ngfx::CullMode::None:  return D3D12_CULL_MODE_NONE;
    case ngfx::CullMode::Front: return D3D12_CULL_MODE_FRONT;
    case ngfx::CullMode::Back:  return D3D12_CULL_MODE_BACK;
    default:                    return D3D12_CULL_MODE_NONE;
    }
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType(ngfx::PrimitiveType p) {
    switch (p) {
    case ngfx::PrimitiveType::Points:         return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case ngfx::PrimitiveType::Lines:
    case ngfx::PrimitiveType::LineStrips:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case ngfx::PrimitiveType::Triangles:
    case ngfx::PrimitiveType::TriangleStrips: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    default:                                  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
}

static D3D_PRIMITIVE_TOPOLOGY primitiveTopology(ngfx::PrimitiveType p) {
    switch (p) {
    case ngfx::PrimitiveType::Points:         return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case ngfx::PrimitiveType::Lines:          return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case ngfx::PrimitiveType::LineStrips:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case ngfx::PrimitiveType::Triangles:      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case ngfx::PrimitiveType::TriangleStrips: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    default:                                  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}

static D3D12_TEXTURE_ADDRESS_MODE addressModeToDX(ngfx::SamplerAddressMode m) {
    switch (m) {
    case ngfx::SamplerAddressMode::Wrap:       return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case ngfx::SamplerAddressMode::Mirror:     return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case ngfx::SamplerAddressMode::Clamp:      return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case ngfx::SamplerAddressMode::Border:     return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    case ngfx::SamplerAddressMode::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
    default:                                   return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

static D3D12_FILTER filterModeToDX(ngfx::FilterMode min, ngfx::FilterMode mag, ngfx::FilterMode mip) {
    int idx = 0;
    if (min == ngfx::FilterMode::Linear) idx |= 0x10;
    if (mag == ngfx::FilterMode::Linear) idx |= 0x04;
    if (mip == ngfx::FilterMode::Linear) idx |= 0x01;
    return (D3D12_FILTER)idx;
}

static D3D12_HEAP_TYPE heapTypeFromStorage(ngfx::StorageMode mode) {
    switch (mode) {
    case ngfx::StorageMode::Shared:    return D3D12_HEAP_TYPE_UPLOAD;
    case ngfx::StorageMode::Private:   return D3D12_HEAP_TYPE_DEFAULT;
    case ngfx::StorageMode::Dedicated: return D3D12_HEAP_TYPE_DEFAULT;
    default:                           return D3D12_HEAP_TYPE_DEFAULT;
    }
}

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
class DxDevice;
class DxBuffer;
class DxTexture;
class DxFactory;

// ---------------------------------------------------------------------------
// DxFence
// ---------------------------------------------------------------------------
class DxFence : public ngfx::Fence {
public:
    DxFence(DxDevice* dev);
    ~DxFence() override;
    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }

    void waitForValue(UINT64 val);
    void signal(ID3D12CommandQueue* queue, UINT64 val);
    UINT64 nextValue() { return ++fence_value_; }

private:
    ComPtr<ID3D12Fence> fence_;
    HANDLE event_ = nullptr;
    UINT64 fence_value_ = 0;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxShader
// ---------------------------------------------------------------------------
class DxShader : public ngfx::Shader {
public:
    DxShader() = default;
    ~DxShader() override = default;
    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
private:
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxBufferView / DxTextureView
// ---------------------------------------------------------------------------
class DxBufferView : public ngfx::BufferView {
public:
    DxBufferView(DxBuffer* b) : buf_(b) {}
    ~DxBufferView() = default;
    const ngfx::Buffer* buffer() const override;
    ngfx::BufferUsage usage() const override;
private:
    DxBuffer* buf_;
};

class DxTextureView : public ngfx::TextureView {
public:
    DxTextureView(DxTexture* t, D3D12_CPU_DESCRIPTOR_HANDLE srv) : tex_(t), srv_(srv) {}
    ~DxTextureView() = default;
    const ngfx::Texture* texture() const override;
    ngfx::TextureUsage usage() const override;
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle() const { return srv_; }
private:
    DxTexture* tex_;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_;
};

// ---------------------------------------------------------------------------
// DxBuffer
// ---------------------------------------------------------------------------
class DxBuffer : public ngfx::Buffer {
public:
    DxBuffer(ID3D12Resource* res, const ngfx::BufferDesc& desc, ngfx::StorageMode mode)
        : resource_(res), desc_(desc), mode_(mode) {}
    ~DxBuffer() override = default;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    void* map(ngfx::uint64 offset, ngfx::uint64 size) override;
    void unmap(void* addr) override;
    ngfx::BufferView* newView(ngfx::Result* result) override;

    ID3D12Resource* resource() const { return resource_.Get(); }
    const ngfx::BufferDesc& desc() const { return desc_; }
    D3D12_GPU_VIRTUAL_ADDRESS gpuVA() const { return resource_->GetGPUVirtualAddress(); }

private:
    ComPtr<ID3D12Resource> resource_;
    ngfx::BufferDesc desc_;
    ngfx::StorageMode mode_;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxTexture
// ---------------------------------------------------------------------------
class DxTexture : public ngfx::Texture {
public:
    DxTexture(ID3D12Resource* res, const ngfx::TextureDesc& desc, DxDevice* dev, bool ownsResource = true)
        : resource_(res), desc_(desc), device_(dev), owns_(ownsResource) {}
    ~DxTexture() override;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    void* map(ngfx::uint64 offset, ngfx::uint64 size) override;
    void unmap(void* addr) override;
    ngfx::PixelFormat format() const override { return desc_.format; }
    ngfx::TextureView* newView(ngfx::Result* result) override;

    ID3D12Resource* resource() const { return resource_.Get(); }
    const ngfx::TextureDesc& desc() const { return desc_; }

private:
    ComPtr<ID3D12Resource> resource_;
    ngfx::TextureDesc desc_;
    DxDevice* device_;
    bool owns_;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxSampler
// ---------------------------------------------------------------------------
class DxSampler : public ngfx::Sampler {
public:
    DxSampler(D3D12_CPU_DESCRIPTOR_HANDLE handle) : handle_(handle) {}
    ~DxSampler() override = default;
    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    D3D12_CPU_DESCRIPTOR_HANDLE handle() const { return handle_; }
private:
    D3D12_CPU_DESCRIPTOR_HANDLE handle_;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxBindGroup
// ---------------------------------------------------------------------------
class DxBindGroup : public ngfx::BindGroup {
public:
    DxBindGroup(DxDevice* dev) : device_(dev) {}
    ~DxBindGroup() override = default;
    void setSampler(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::Sampler* sampler) override {}
    void setTexture(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::TextureView* texture) override {}
    void setBuffer(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::BufferView* buffer) override {}
    void setRaytracingAS(ngfx::uint32 id, ngfx::ShaderStage stage, const ngfx::RaytracingAS* as) override {}
private:
    DxDevice* device_;
};

// ---------------------------------------------------------------------------
// DxRaytracingAS
// ---------------------------------------------------------------------------
class DxRaytracingAS : public ngfx::RaytracingAS {
public:
    DxRaytracingAS() = default;
    ~DxRaytracingAS() override = default;
    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
private:
    std::string label_;
};

// ---------------------------------------------------------------------------
// Shared D3D12 pipeline state (composition helper, not a base class)
// ---------------------------------------------------------------------------
struct DxPipelineState {
    ComPtr<ID3D12PipelineState> pso;
    ComPtr<ID3D12RootSignature> root_sig;

    ID3D12PipelineState* getPSO() const { return pso.Get(); }
    ID3D12RootSignature* getRootSig() const { return root_sig.Get(); }

    void createEmptyRootSignature(ID3D12Device* dev) {
        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        ComPtr<ID3DBlob> blob, err;
        D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err);
        if (blob) {
            dev->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_sig));
        }
    }
};

// ---------------------------------------------------------------------------
// DxRenderPipeline
// ---------------------------------------------------------------------------
class DxRenderPipeline : public ngfx::RenderPipeline {
public:
    DxRenderPipeline(DxDevice* dev, const ngfx::RenderPipelineDesc& desc, DXGI_FORMAT rtvFormat);
    ~DxRenderPipeline() override = default;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    ngfx::BindGroup* newBindGroup(ngfx::Result* result) override;
    ngfx::Device* device() override;

    ID3D12PipelineState* pso() const { return ps_.getPSO(); }
    ID3D12RootSignature* rootSig() const { return ps_.getRootSig(); }

private:
    DxPipelineState ps_;
    DxDevice* device_;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxComputePipeline
// ---------------------------------------------------------------------------
class DxComputePipeline : public ngfx::ComputePipeline {
public:
    DxComputePipeline(DxDevice* dev, const ngfx::ComputePipelineDesc& desc);
    ~DxComputePipeline() override = default;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    ngfx::BindGroup* newBindGroup(ngfx::Result* result) override;
    ngfx::Device* device() override;

    ID3D12PipelineState* pso() const { return ps_.getPSO(); }
    ID3D12RootSignature* rootSig() const { return ps_.getRootSig(); }

private:
    DxPipelineState ps_;
    DxDevice* device_;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxRenderpass
// ---------------------------------------------------------------------------
class DxRenderpass : public ngfx::Renderpass {
public:
    DxRenderpass(DxDevice* dev, DXGI_FORMAT rtvFormat)
        : device_(dev), rtv_format_(rtvFormat) {}
    ~DxRenderpass() override = default;
    ngfx::RenderPipeline* newRenderPipeline(const ngfx::RenderPipelineDesc* desc, ngfx::Result* result) override;

    DXGI_FORMAT rtvFormat() const { return rtv_format_; }

private:
    DxDevice* device_;
    DXGI_FORMAT rtv_format_;
};

// ---------------------------------------------------------------------------
// DxCommandBuffer / Encoders
// ---------------------------------------------------------------------------
class DxCommandBuffer;

class DxRenderEncoder : public ngfx::RenderEncoder {
public:
    DxRenderEncoder(DxCommandBuffer* cmd);
    ~DxRenderEncoder() override = default;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    void setPipeline(ngfx::Pipeline* pipeline) override;
    void setBindGroup(const ngfx::BindGroup* bindGroup) override {}
    void setViewport(ngfx::Viewport vp) override;
    void setViewports(int n, const ngfx::Viewport* vps) override;
    void setScissors(int n, const ngfx::Rect* rects) override;
    void setStencilRef() override;
    void setDepthBias() override;
    void drawPrimitives(ngfx::PrimitiveType pt, int vertexStart, int vertexCount, int instanceCount, int baseInstance) override;
    void drawIndexedPrimitives(ngfx::PrimitiveType pt, ngfx::IndexType it, int indexCount,
        const ngfx::Buffer* indexBuf, int indexBufOffset,
        int vertexStart, int vertexCount, int instanceCount, int baseInstance) override;
    void drawIndirect(ngfx::PrimitiveType pt, const ngfx::Buffer* buf, ngfx::uint64 offset, ngfx::uint32 drawCount, ngfx::uint32 stride) override;
    void present(ngfx::Drawable* drawable) override;
    void endEncode() override;

private:
    ID3D12GraphicsCommandList* list();
    DxCommandBuffer* cmd_;
    ngfx::Drawable* pending_drawable_ = nullptr;
    std::string label_;
};

class DxComputeEncoder : public ngfx::ComputeEncoder {
public:
    DxComputeEncoder(DxCommandBuffer* cmd) : cmd_(cmd) {}
    ~DxComputeEncoder() override = default;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    void setPipeline(ngfx::Pipeline* pipeline) override;
    void setBindGroup(const ngfx::BindGroup* bindGroup) override {}
    void dispatch(int x, int y, int z) override;
    void endEncode() override {}

private:
    ID3D12GraphicsCommandList* list();
    DxCommandBuffer* cmd_;
    std::string label_;
};

class DxRaytraceEncoder : public ngfx::RaytraceEncoder {
public:
    DxRaytraceEncoder() = default;
    ~DxRaytraceEncoder() override = default;
    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    void setPipeline(ngfx::Pipeline*) override {}
    void setBindGroup(const ngfx::BindGroup*) override {}
    void endEncode() override {}
    void buildAS(ngfx::RaytracingAS*, ngfx::RaytracingAS*, ngfx::Buffer*) override {}
    void copyAS(ngfx::RaytracingAS*, ngfx::RaytracingAS*, ngfx::AccelerationStructureCopyMode) override {}
    void traceRay(ngfx::Buffer*, ngfx::BufferStride, ngfx::BufferStride, int, int) override {}
private:
    std::string label_;
};

class DxParallelEncoder : public ngfx::ParallelEncoder {
public:
    DxParallelEncoder(DxCommandBuffer* cmd) : cmd_(cmd) {}
    ~DxParallelEncoder() override = default;
    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    void setPipeline(ngfx::Pipeline*) override {}
    void setBindGroup(const ngfx::BindGroup*) override {}
    void endEncode() override {}
    ngfx::RenderEncoder* subRenderEncoder(ngfx::Result* result) override;
private:
    DxCommandBuffer* cmd_;
    std::string label_;
};

class DxCommandBuffer : public ngfx::CommandBuffer {
public:
    DxCommandBuffer(ID3D12Device* dev, ID3D12CommandQueue* queue);
    ~DxCommandBuffer() override;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }

    ngfx::RenderEncoder* newRenderEncoder(ngfx::Result* result) override;
    ngfx::ComputeEncoder* newComputeEncoder(ngfx::Result* result) override;
    ngfx::BlitEncoder* newBlitEncoder(ngfx::Result* result) override { return nullptr; }
    ngfx::ParallelEncoder* newParallelRenderEncoder(ngfx::Result* result) override;
    ngfx::RaytraceEncoder* newRaytraceEncoder(ngfx::Result* result) override;
    ngfx::Result commit() override;

    ID3D12GraphicsCommandList* commandList() const { return cmd_list_.Get(); }
    ID3D12CommandQueue* queue() const { return queue_; }

private:
    ComPtr<ID3D12CommandAllocator> allocator_;
    ComPtr<ID3D12GraphicsCommandList> cmd_list_;
    ID3D12CommandQueue* queue_;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxQueue
// ---------------------------------------------------------------------------
class DxQueue : public ngfx::CommandQueue {
public:
    DxQueue(ID3D12Device* dev, D3D12_COMMAND_LIST_TYPE type);
    ~DxQueue() override = default;
    ngfx::CommandBuffer* newCommandBuffer() override;

    ID3D12CommandQueue* dxQueue() const { return queue_.Get(); }
    ID3D12Device* dxDevice() const { return device_; }

private:
    ComPtr<ID3D12CommandQueue> queue_;
    ID3D12Device* device_;
};

// ---------------------------------------------------------------------------
// DxDrawable / DxPresentLayer
// ---------------------------------------------------------------------------
class DxPresentLayer;

class DxDrawable : public ngfx::Drawable {
public:
    DxDrawable(DxPresentLayer* layer, UINT index, ID3D12Resource* backBuf);
    ~DxDrawable() override;
    int drawableId() const override { return (int)index_; }
    ngfx::Texture* texture() override;
    ngfx::PresentLayer* layer() override;
    void present() override;

private:
    DxPresentLayer* layer_;
    UINT index_;
    ID3D12Resource* back_buffer_;
    DxTexture* tex_wrapper_ = nullptr;
};

class DxPresentLayer : public ngfx::PresentLayer {
public:
    DxPresentLayer(DxDevice* dev, IDXGISwapChain3* swapchain, DXGI_FORMAT fmt, UINT w, UINT h, UINT imageCount);
    ~DxPresentLayer() override;

    void getDesc(ngfx::PresentLayerDesc* desc) const override;
    ngfx::Device* device() override;
    ngfx::Drawable* nextDrawable() override;

    IDXGISwapChain3* swapchain() const { return swapchain_.Get(); }
    DxDevice* dxDevice() const { return device_; }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(UINT index) const;

private:
    DxDevice* device_;
    ComPtr<IDXGISwapChain3> swapchain_;
    ComPtr<ID3D12DescriptorHeap> rtv_heap_;
    std::vector<ComPtr<ID3D12Resource>> back_buffers_;
    DXGI_FORMAT format_;
    UINT width_, height_, image_count_;
    UINT rtv_increment_;
};

// ---------------------------------------------------------------------------
// DxDevice
// ---------------------------------------------------------------------------
class DxDevice : public ngfx::Device {
public:
    DxDevice(IDXGIAdapter1* adapter, ngfx_LogCallback log);
    ~DxDevice() override;

    void setLabel(const char* l) override { if (l) label_ = l; }
    const char* label() const override { return label_.empty() ? nullptr : label_.c_str(); }
    ngfx::DeviceType getType() const override { return type_; }

    ngfx::CommandQueue* newQueue(ngfx::Result* result) override;
    ngfx::Shader* newShader() override;
    ngfx::Renderpass* newRenderpass(const ngfx::RenderpassDesc* desc, ngfx::Result* result) override;
    ngfx::ComputePipeline* newComputePipeline(const ngfx::ComputePipelineDesc* desc, ngfx::Result* result) override;
    ngfx::RaytracePipeline* newRaytracePipeline(const ngfx::RaytracePipelineDesc* desc, ngfx::Result* result) override;
    ngfx::Texture* newTexture(const ngfx::TextureDesc* desc, ngfx::StorageMode mode, ngfx::Result* result) override;
    ngfx::Buffer* newBuffer(const ngfx::BufferDesc* desc, ngfx::StorageMode mode, ngfx::Result* result) override;
    ngfx::RaytracingAS* newRaytracingAS(const ngfx::RaytracingASDesc* rtDesc, ngfx::Result* result) override;
    ngfx::Sampler* newSampler(const ngfx::SamplerDesc* desc, ngfx::Result* result) override;
    ngfx::Fence* newFence(ngfx::Result* result) override;
    ngfx::Result wait() override;

    ID3D12Device* dxDevice() const { return device_.Get(); }
    bool isValid() const { return device_.Get() != nullptr; }
    ID3D12CommandQueue* directQueue();

private:
    ComPtr<ID3D12Device> device_;
    ComPtr<IDXGIAdapter1> adapter_;
    ComPtr<ID3D12CommandQueue> direct_queue_;
    ComPtr<ID3D12DescriptorHeap> sampler_heap_;
    UINT sampler_offset_ = 0;
    UINT sampler_increment_ = 0;
    ngfx::DeviceType type_ = ngfx::DeviceType::Discrete;
    ngfx_LogCallback log_ = nullptr;
    std::string label_;
};

// ---------------------------------------------------------------------------
// DxFactory
// ---------------------------------------------------------------------------
class DxFactory : public ngfx::Factory {
public:
    DxFactory(bool debug, ngfx_LogCallback log);
    ~DxFactory() override;

    int numDevices() override { return (int)devices_.size(); }
    ngfx::Device* getDevice(ngfx::uint32 id) override { return id < devices_.size() ? devices_[id] : nullptr; }
    ngfx::PresentLayer* newPresentLayer(const ngfx::PresentLayerDesc* desc, ngfx::Device* dev, ngfx::PresentLayer* old, ngfx::Result* result) override;

private:
    ComPtr<IDXGIFactory4> factory_;
    std::vector<DxDevice*> devices_;
    ngfx_LogCallback log_;
};

// ===================================================================
// Implementation
// ===================================================================

// ---------------------------------------------------------------------------
// DxFence
// ---------------------------------------------------------------------------
DxFence::DxFence(DxDevice* dev) {
    dev->dxDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    event_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
}

DxFence::~DxFence() {
    if (event_) CloseHandle(event_);
}

void DxFence::waitForValue(UINT64 val) {
    if (fence_->GetCompletedValue() < val) {
        fence_->SetEventOnCompletion(val, event_);
        WaitForSingleObject(event_, INFINITE);
    }
}

void DxFence::signal(ID3D12CommandQueue* queue, UINT64 val) {
    queue->Signal(fence_.Get(), val);
}

// ---------------------------------------------------------------------------
// DxBuffer
// ---------------------------------------------------------------------------
void* DxBuffer::map(ngfx::uint64 offset, ngfx::uint64 size) {
    if (mode_ != ngfx::StorageMode::Shared) return nullptr;
    D3D12_RANGE range = { (SIZE_T)offset, (SIZE_T)(offset + size) };
    void* ptr = nullptr;
    HRESULT hr = resource_->Map(0, &range, &ptr);
    return SUCCEEDED(hr) ? (uint8_t*)ptr + offset : nullptr;
}

void DxBuffer::unmap(void* addr) {
    resource_->Unmap(0, nullptr);
}

ngfx::BufferView* DxBuffer::newView(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxBufferView(this);
}

const ngfx::Buffer* DxBufferView::buffer() const { return buf_; }
ngfx::BufferUsage DxBufferView::usage() const { return buf_->desc().usages; }

// ---------------------------------------------------------------------------
// DxTexture
// ---------------------------------------------------------------------------
DxTexture::~DxTexture() {
    if (!owns_) resource_.Detach();
}

void* DxTexture::map(ngfx::uint64 offset, ngfx::uint64 size) {
    return nullptr;
}

void DxTexture::unmap(void* addr) {}

ngfx::TextureView* DxTexture::newView(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
    return new DxTextureView(this, handle);
}

const ngfx::Texture* DxTextureView::texture() const { return tex_; }
ngfx::TextureUsage DxTextureView::usage() const { return tex_->desc().usages; }

// ---------------------------------------------------------------------------
// DxRenderPipeline / DxComputePipeline shared
// ---------------------------------------------------------------------------
ngfx::BindGroup* DxRenderPipeline::newBindGroup(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxBindGroup(device_);
}
ngfx::Device* DxRenderPipeline::device() { return device_; }

ngfx::BindGroup* DxComputePipeline::newBindGroup(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxBindGroup(device_);
}
ngfx::Device* DxComputePipeline::device() { return device_; }

// ---------------------------------------------------------------------------
// DxRenderPipeline
// ---------------------------------------------------------------------------
DxRenderPipeline::DxRenderPipeline(DxDevice* dev, const ngfx::RenderPipelineDesc& desc, DXGI_FORMAT rtvFormat)
    : device_(dev)
{
    ps_.createEmptyRootSignature(dev->dxDevice());
    if (!ps_.root_sig) return;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = ps_.root_sig.Get();

    if (desc.vertex && desc.vertex->bundle()) {
        pso.VS.pShaderBytecode = desc.vertex->bundle()->data();
        pso.VS.BytecodeLength = (SIZE_T)desc.vertex->bundle()->length();
    }
    if (desc.pixel && desc.pixel->bundle()) {
        pso.PS.pShaderBytecode = desc.pixel->bundle()->data();
        pso.PS.BytecodeLength = (SIZE_T)desc.pixel->bundle()->length();
    }
    if (desc.geometry && desc.geometry->bundle()) {
        pso.GS.pShaderBytecode = desc.geometry->bundle()->data();
        pso.GS.BytecodeLength = (SIZE_T)desc.geometry->bundle()->length();
    }
    if (desc.domain && desc.domain->bundle()) {
        pso.DS.pShaderBytecode = desc.domain->bundle()->data();
        pso.DS.BytecodeLength = (SIZE_T)desc.domain->bundle()->length();
    }
    if (desc.hull && desc.hull->bundle()) {
        pso.HS.pShaderBytecode = desc.hull->bundle()->data();
        pso.HS.BytecodeLength = (SIZE_T)desc.hull->bundle()->length();
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> elems;
    for (size_t i = 0; i < desc.input.attributes.num(); ++i) {
        D3D12_INPUT_ELEMENT_DESC e = {};
        e.SemanticName = "ATTR";
        e.SemanticIndex = (UINT)i;
        e.Format = vertexFormatToDXGI(desc.input.attributes.at(i).format);
        e.InputSlot = desc.input.attributes.at(i).slot;
        e.AlignedByteOffset = desc.input.attributes.at(i).offset;
        e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        if (desc.input.layouts.num() > desc.input.attributes.at(i).slot &&
            desc.input.layouts.at(desc.input.attributes.at(i).slot).inputRate == ngfx::VertexInputRate::PerInstance) {
            e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            e.InstanceDataStepRate = 1;
        }
        elems.push_back(e);
    }
    pso.InputLayout.pInputElementDescs = elems.data();
    pso.InputLayout.NumElements = (UINT)elems.size();

    pso.RasterizerState.FillMode = fillModeToDX(desc.rasterizer.fillMode);
    pso.RasterizerState.CullMode = cullModeToDX(desc.rasterizer.cullMode);
    pso.RasterizerState.FrontCounterClockwise = desc.rasterizer.frontCCW ? TRUE : FALSE;
    pso.RasterizerState.DepthBias = (INT)desc.rasterizer.depthBias;
    pso.RasterizerState.DepthBiasClamp = desc.rasterizer.depthBiasClamp;
    pso.RasterizerState.SlopeScaledDepthBias = desc.rasterizer.depthBiasSlope;
    pso.RasterizerState.DepthClipEnable = desc.rasterizer.depthClipEnable ? TRUE : FALSE;
    pso.RasterizerState.MultisampleEnable = FALSE;
    pso.RasterizerState.AntialiasedLineEnable = FALSE;
    pso.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    pso.DepthStencilState.DepthEnable = desc.depthStencil.depthTest ? TRUE : FALSE;
    pso.DepthStencilState.DepthWriteMask = desc.depthStencil.depthWriteMask == ngfx::DepthWriteMask::All ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    pso.DepthStencilState.DepthFunc = comparisonToDX(desc.depthStencil.depthFunction);
    pso.DepthStencilState.StencilEnable = desc.depthStencil.stencilTest ? TRUE : FALSE;
    pso.DepthStencilState.StencilReadMask = 0xFF;
    pso.DepthStencilState.StencilWriteMask = 0xFF;
    pso.DepthStencilState.FrontFace.StencilFailOp = stencilOpToDX(desc.depthStencil.front.stencilFailOp);
    pso.DepthStencilState.FrontFace.StencilDepthFailOp = stencilOpToDX(desc.depthStencil.front.depthStencilFailOp);
    pso.DepthStencilState.FrontFace.StencilPassOp = stencilOpToDX(desc.depthStencil.front.stencilPassOp);
    pso.DepthStencilState.FrontFace.StencilFunc = comparisonToDX(desc.depthStencil.front.stencilFunc);
    pso.DepthStencilState.BackFace.StencilFailOp = stencilOpToDX(desc.depthStencil.back.stencilFailOp);
    pso.DepthStencilState.BackFace.StencilDepthFailOp = stencilOpToDX(desc.depthStencil.back.depthStencilFailOp);
    pso.DepthStencilState.BackFace.StencilPassOp = stencilOpToDX(desc.depthStencil.back.stencilPassOp);
    pso.DepthStencilState.BackFace.StencilFunc = comparisonToDX(desc.depthStencil.back.stencilFunc);

    pso.BlendState.AlphaToCoverageEnable = FALSE;
    pso.BlendState.IndependentBlendEnable = (desc.blend.renderTargets.num() > 1) ? TRUE : FALSE;
    for (size_t i = 0; i < desc.blend.renderTargets.num() && i < 8; ++i) {
        auto& src = desc.blend.renderTargets.at(i);
        auto& dst = pso.BlendState.RenderTarget[i];
        dst.BlendEnable = src.blendEnable ? TRUE : FALSE;
        dst.SrcBlend = blendFactorToDX(src.srcColor);
        dst.DestBlend = blendFactorToDX(src.destColor);
        dst.BlendOp = blendOpToDX(src.colorOp);
        dst.SrcBlendAlpha = blendFactorToDX(src.srcAlpha);
        dst.DestBlendAlpha = blendFactorToDX(src.destAlpha);
        dst.BlendOpAlpha = blendOpToDX(src.alphaOp);
        dst.RenderTargetWriteMask = (UINT8)(src.colorWriteMask & 0xF);
    }
    if (desc.blend.renderTargets.num() == 0) {
        pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    pso.SampleMask = UINT_MAX;
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = rtvFormat;
    pso.DSVFormat = pixelFormatToDXGI(desc.depthStencilFormat);
    pso.SampleDesc.Count = 1;

    dev->dxDevice()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&ps_.pso));
}

// ---------------------------------------------------------------------------
// DxComputePipeline
// ---------------------------------------------------------------------------
DxComputePipeline::DxComputePipeline(DxDevice* dev, const ngfx::ComputePipelineDesc& desc)
    : device_(dev)
{
    ps_.createEmptyRootSignature(dev->dxDevice());
    if (!ps_.root_sig) return;
    if (!desc.function || !desc.function->bundle()) return;

    D3D12_COMPUTE_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = ps_.root_sig.Get();
    pso.CS.pShaderBytecode = desc.function->bundle()->data();
    pso.CS.BytecodeLength = (SIZE_T)desc.function->bundle()->length();
    dev->dxDevice()->CreateComputePipelineState(&pso, IID_PPV_ARGS(&ps_.pso));
}

// ---------------------------------------------------------------------------
// DxRenderpass
// ---------------------------------------------------------------------------
ngfx::RenderPipeline* DxRenderpass::newRenderPipeline(const ngfx::RenderPipelineDesc* desc, ngfx::Result* result) {
    auto* p = new DxRenderPipeline(device_, *desc, rtv_format_);
    if (result) *result = ngfx::Result::Ok;
    return p;
}

// ---------------------------------------------------------------------------
// DxCommandBuffer
// ---------------------------------------------------------------------------
DxCommandBuffer::DxCommandBuffer(ID3D12Device* dev, ID3D12CommandQueue* queue)
    : queue_(queue)
{
    dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator_));
    dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator_.Get(), nullptr, IID_PPV_ARGS(&cmd_list_));
}

DxCommandBuffer::~DxCommandBuffer() {}

ngfx::RenderEncoder* DxCommandBuffer::newRenderEncoder(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxRenderEncoder(this);
}

ngfx::ComputeEncoder* DxCommandBuffer::newComputeEncoder(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxComputeEncoder(this);
}

ngfx::ParallelEncoder* DxCommandBuffer::newParallelRenderEncoder(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxParallelEncoder(this);
}

ngfx::RaytraceEncoder* DxCommandBuffer::newRaytraceEncoder(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxRaytraceEncoder();
}

ngfx::Result DxCommandBuffer::commit() {
    HRESULT hr = cmd_list_->Close();
    if (FAILED(hr)) return ngfx::Result::Failed;
    ID3D12CommandList* lists[] = { cmd_list_.Get() };
    queue_->ExecuteCommandLists(1, lists);
    return ngfx::Result::Ok;
}

// ---------------------------------------------------------------------------
// DxRenderEncoder
// ---------------------------------------------------------------------------
DxRenderEncoder::DxRenderEncoder(DxCommandBuffer* cmd) : cmd_(cmd) {}

ID3D12GraphicsCommandList* DxRenderEncoder::list() { return cmd_->commandList(); }

void DxRenderEncoder::setPipeline(ngfx::Pipeline* pipeline) {
    if (!pipeline) return;
    auto* dx = static_cast<DxRenderPipeline*>(pipeline);
    list()->SetPipelineState(dx->pso());
    list()->SetGraphicsRootSignature(dx->rootSig());
    list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxRenderEncoder::setViewport(ngfx::Viewport vp) {
    D3D12_VIEWPORT d = { vp.x, vp.y, vp.width, vp.height, vp.near, vp.far };
    list()->RSSetViewports(1, &d);
}

void DxRenderEncoder::setViewports(int n, const ngfx::Viewport* vps) {
    std::vector<D3D12_VIEWPORT> d(n);
    for (int i = 0; i < n; ++i) {
        d[i] = { vps[i].x, vps[i].y, vps[i].width, vps[i].height, vps[i].near, vps[i].far };
    }
    list()->RSSetViewports((UINT)n, d.data());
}

void DxRenderEncoder::setScissors(int n, const ngfx::Rect* rects) {
    std::vector<D3D12_RECT> d(n);
    for (int i = 0; i < n; ++i) {
        d[i] = { (LONG)rects[i].x, (LONG)rects[i].y, (LONG)(rects[i].x + rects[i].width), (LONG)(rects[i].y + rects[i].height) };
    }
    list()->RSSetScissorRects((UINT)n, d.data());
}

void DxRenderEncoder::setStencilRef() { list()->OMSetStencilRef(0); }
void DxRenderEncoder::setDepthBias() {}

void DxRenderEncoder::drawPrimitives(ngfx::PrimitiveType pt, int vertexStart, int vertexCount, int instanceCount, int baseInstance) {
    list()->IASetPrimitiveTopology(primitiveTopology(pt));
    list()->DrawInstanced(vertexCount, instanceCount > 0 ? instanceCount : 1, vertexStart, baseInstance);
}

void DxRenderEncoder::drawIndexedPrimitives(ngfx::PrimitiveType pt, ngfx::IndexType it, int indexCount,
    const ngfx::Buffer* indexBuf, int indexBufOffset,
    int vertexStart, int vertexCount, int instanceCount, int baseInstance)
{
    if (!indexBuf) return;
    auto* buf = static_cast<const DxBuffer*>(indexBuf);
    D3D12_INDEX_BUFFER_VIEW ibv = {};
    ibv.BufferLocation = buf->gpuVA() + indexBufOffset;
    ibv.SizeInBytes = buf->desc().size - indexBufOffset;
    ibv.Format = (it == ngfx::IndexType::UInt16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    list()->IASetIndexBuffer(&ibv);
    list()->IASetPrimitiveTopology(primitiveTopology(pt));
    list()->DrawIndexedInstanced(indexCount, instanceCount > 0 ? instanceCount : 1, 0, vertexStart, baseInstance);
}

void DxRenderEncoder::drawIndirect(ngfx::PrimitiveType pt, const ngfx::Buffer* buf, ngfx::uint64 offset, ngfx::uint32 drawCount, ngfx::uint32 stride) {
    list()->IASetPrimitiveTopology(primitiveTopology(pt));
}

void DxRenderEncoder::present(ngfx::Drawable* drawable) { pending_drawable_ = drawable; }

void DxRenderEncoder::endEncode() {
    if (pending_drawable_) {
        pending_drawable_->present();
        pending_drawable_ = nullptr;
    }
}

// ---------------------------------------------------------------------------
// DxComputeEncoder
// ---------------------------------------------------------------------------
ID3D12GraphicsCommandList* DxComputeEncoder::list() { return cmd_->commandList(); }

void DxComputeEncoder::setPipeline(ngfx::Pipeline* pipeline) {
    if (!pipeline) return;
    auto* dx = static_cast<DxComputePipeline*>(pipeline);
    list()->SetPipelineState(dx->pso());
    list()->SetComputeRootSignature(dx->rootSig());
}

void DxComputeEncoder::dispatch(int x, int y, int z) {
    list()->Dispatch((UINT)x, (UINT)y, (UINT)z);
}

// ---------------------------------------------------------------------------
// DxParallelEncoder
// ---------------------------------------------------------------------------
ngfx::RenderEncoder* DxParallelEncoder::subRenderEncoder(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxRenderEncoder(cmd_);
}

// ---------------------------------------------------------------------------
// DxQueue
// ---------------------------------------------------------------------------
DxQueue::DxQueue(ID3D12Device* dev, D3D12_COMMAND_LIST_TYPE type)
    : device_(dev)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue_));
}

ngfx::CommandBuffer* DxQueue::newCommandBuffer() {
    return new DxCommandBuffer(device_, queue_.Get());
}

// ---------------------------------------------------------------------------
// DxPresentLayer / DxDrawable
// ---------------------------------------------------------------------------
DxPresentLayer::DxPresentLayer(DxDevice* dev, IDXGISwapChain3* sc, DXGI_FORMAT fmt, UINT w, UINT h, UINT count)
    : device_(dev), format_(fmt), width_(w), height_(h), image_count_(count)
{
    swapchain_.Attach(sc);

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = count;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    dev->dxDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtv_heap_));
    rtv_increment_ = dev->dxDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    back_buffers_.resize(count);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < count; ++i) {
        sc->GetBuffer(i, IID_PPV_ARGS(&back_buffers_[i]));
        dev->dxDevice()->CreateRenderTargetView(back_buffers_[i].Get(), nullptr, handle);
        handle.ptr += rtv_increment_;
    }
}

DxPresentLayer::~DxPresentLayer() {}

void DxPresentLayer::getDesc(ngfx::PresentLayerDesc* desc) const {
    if (!desc) return;
    desc->format = dxgiToPixelFormat(format_);
    desc->width = width_;
    desc->height = height_;
    desc->colorSpace = ngfx::ColorSpace::SRGBNonLinear;
    desc->maxImages = image_count_;
}

ngfx::Device* DxPresentLayer::device() { return device_; }

D3D12_CPU_DESCRIPTOR_HANDLE DxPresentLayer::rtvHandle(UINT index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE h = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    h.ptr += (SIZE_T)index * rtv_increment_;
    return h;
}

ngfx::Drawable* DxPresentLayer::nextDrawable() {
    UINT idx = swapchain_->GetCurrentBackBufferIndex();
    return new DxDrawable(this, idx, back_buffers_[idx].Get());
}

DxDrawable::DxDrawable(DxPresentLayer* layer, UINT index, ID3D12Resource* bb)
    : layer_(layer), index_(index), back_buffer_(bb) {}

DxDrawable::~DxDrawable() { delete tex_wrapper_; }

ngfx::Texture* DxDrawable::texture() {
    if (!tex_wrapper_) {
        ngfx::PresentLayerDesc ld = {};
        layer_->getDesc(&ld);
        ngfx::TextureDesc td = {};
        td.format = ld.format;
        td.width = ld.width;
        td.height = ld.height;
        td.depth = 1;
        td.layers = 1;
        td.mipLevels = 1;
        td.usages = ngfx::TextureUsage::RenderTarget;
        tex_wrapper_ = new DxTexture(back_buffer_, td, layer_->dxDevice(), false);
    }
    return tex_wrapper_;
}

ngfx::PresentLayer* DxDrawable::layer() { return layer_; }

void DxDrawable::present() {
    layer_->swapchain()->Present(1, 0);
}

// ---------------------------------------------------------------------------
// DxDevice
// ---------------------------------------------------------------------------
DxDevice::DxDevice(IDXGIAdapter1* adapter, ngfx_LogCallback log)
    : log_(log)
{
    adapter_.Attach(adapter);
    adapter->AddRef();

    DXGI_ADAPTER_DESC1 adapterDesc;
    adapter->GetDesc1(&adapterDesc);

    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        type_ = ngfx::DeviceType::Virtual;
    else
        type_ = ngfx::DeviceType::Discrete;

    char name[256];
    WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, -1, name, 256, nullptr, nullptr);
    label_ = name;

    HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
    }

    if (SUCCEEDED(hr) && device_) {
        D3D12_COMMAND_QUEUE_DESC qd = {};
        qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        device_->CreateCommandQueue(&qd, IID_PPV_ARGS(&direct_queue_));

        D3D12_DESCRIPTOR_HEAP_DESC shd = {};
        shd.NumDescriptors = 256;
        shd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        device_->CreateDescriptorHeap(&shd, IID_PPV_ARGS(&sampler_heap_));
        sampler_increment_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
}

DxDevice::~DxDevice() {}

ID3D12CommandQueue* DxDevice::directQueue() {
    return direct_queue_.Get();
}

ngfx::CommandQueue* DxDevice::newQueue(ngfx::Result* result) {
    auto* q = new DxQueue(device_.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    if (result) *result = ngfx::Result::Ok;
    return q;
}

ngfx::Shader* DxDevice::newShader() { return new DxShader(); }

ngfx::Renderpass* DxDevice::newRenderpass(const ngfx::RenderpassDesc* desc, ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxRenderpass(this, DXGI_FORMAT_B8G8R8A8_UNORM);
}

ngfx::ComputePipeline* DxDevice::newComputePipeline(const ngfx::ComputePipelineDesc* desc, ngfx::Result* result) {
    auto* p = new DxComputePipeline(this, *desc);
    if (result) *result = ngfx::Result::Ok;
    return p;
}

ngfx::RaytracePipeline* DxDevice::newRaytracePipeline(const ngfx::RaytracePipelineDesc*, ngfx::Result* result) {
    if (result) *result = ngfx::Result::Failed;
    return nullptr;
}

ngfx::Texture* DxDevice::newTexture(const ngfx::TextureDesc* desc, ngfx::StorageMode mode, ngfx::Result* result) {
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = (desc->depth > 1) ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Width = desc->width > 0 ? desc->width : 1;
    rd.Height = desc->height > 0 ? desc->height : 1;
    rd.DepthOrArraySize = (desc->depth > 1) ? (UINT16)desc->depth : (desc->layers > 0 ? (UINT16)desc->layers : 1);
    rd.MipLevels = (UINT16)(desc->mipLevels > 0 ? desc->mipLevels : 1);
    rd.Format = pixelFormatToDXGI(desc->format);
    rd.SampleDesc.Count = 1;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;
    if ((uint8_t)desc->usages & (uint8_t)ngfx::TextureUsage::RenderTarget)
        rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if ((uint8_t)desc->usages & (uint8_t)ngfx::TextureUsage::DepthStencil)
        rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = heapTypeFromStorage(mode);

    ComPtr<ID3D12Resource> resource;
    HRESULT hr = device_->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &rd,
        D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource));
    if (SUCCEEDED(hr)) {
        if (result) *result = ngfx::Result::Ok;
        return new DxTexture(resource.Detach(), *desc, this);
    }
    if (result) *result = ngfx::Result::Failed;
    return nullptr;
}

ngfx::Buffer* DxDevice::newBuffer(const ngfx::BufferDesc* desc, ngfx::StorageMode mode, ngfx::Result* result) {
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Width = desc->size > 0 ? desc->size : 256;
    rd.Height = 1;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_UNKNOWN;
    rd.SampleDesc.Count = 1;
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;
    if ((uint8_t)desc->usages & (uint8_t)ngfx::BufferUsage::UnorderedAccess)
        rd.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = heapTypeFromStorage(mode);
    D3D12_RESOURCE_STATES initState = (heap.Type == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

    ComPtr<ID3D12Resource> resource;
    HRESULT hr = device_->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &rd,
        initState, nullptr, IID_PPV_ARGS(&resource));
    if (SUCCEEDED(hr)) {
        if (result) *result = ngfx::Result::Ok;
        return new DxBuffer(resource.Detach(), *desc, mode);
    }
    if (result) *result = ngfx::Result::Failed;
    return nullptr;
}

ngfx::RaytracingAS* DxDevice::newRaytracingAS(const ngfx::RaytracingASDesc*, ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxRaytracingAS();
}

ngfx::Sampler* DxDevice::newSampler(const ngfx::SamplerDesc* desc, ngfx::Result* result) {
    D3D12_SAMPLER_DESC sd = {};
    sd.Filter = filterModeToDX(desc->minFilter, desc->magFilter, desc->mipMapFilter);
    sd.AddressU = addressModeToDX(desc->u);
    sd.AddressV = addressModeToDX(desc->v);
    sd.AddressW = addressModeToDX(desc->w);
    sd.MipLODBias = desc->minLodBias;
    sd.MaxAnisotropy = desc->maxAnistropy;
    sd.ComparisonFunc = comparisonToDX(desc->comparisonFunction);
    sd.MinLOD = desc->minLod;
    sd.MaxLOD = desc->maxLod;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = sampler_heap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)sampler_offset_ * sampler_increment_;
    device_->CreateSampler(&sd, handle);
    sampler_offset_++;

    if (result) *result = ngfx::Result::Ok;
    return new DxSampler(handle);
}

ngfx::Fence* DxDevice::newFence(ngfx::Result* result) {
    if (result) *result = ngfx::Result::Ok;
    return new DxFence(this);
}

ngfx::Result DxDevice::wait() {
    if (!direct_queue_) return ngfx::Result::Failed;
    DxFence fence(this);
    UINT64 val = fence.nextValue();
    fence.signal(direct_queue_.Get(), val);
    fence.waitForValue(val);
    return ngfx::Result::Ok;
}

// ---------------------------------------------------------------------------
// DxFactory
// ---------------------------------------------------------------------------
DxFactory::DxFactory(bool debug, ngfx_LogCallback log)
    : log_(log)
{
    UINT dxgiFlags = 0;
    if (debug) {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
            dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&factory_));
    if (!factory_) return;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory_->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            adapter.Reset();
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
            auto* dev = new DxDevice(adapter.Get(), log);
            if (dev->isValid())
                devices_.push_back(dev);
            else
                delete dev;
        }
        adapter.Reset();
    }
}

DxFactory::~DxFactory() {
    for (auto* d : devices_) delete d;
    devices_.clear();
}

ngfx::PresentLayer* DxFactory::newPresentLayer(const ngfx::PresentLayerDesc* desc, ngfx::Device* dev, ngfx::PresentLayer* old, ngfx::Result* result) {
    auto* dxDev = static_cast<DxDevice*>(dev);
    DXGI_FORMAT fmt = pixelFormatToDXGI(desc->format);
    UINT count = desc->maxImages > 0 ? desc->maxImages : 3;

    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.Width = desc->width;
    scd.Height = desc->height;
    scd.Format = fmt;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = count;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> sc1;
    HRESULT hr = factory_->CreateSwapChainForHwnd(
        dxDev->directQueue(),
        (HWND)desc->winHandle,
        &scd, nullptr, nullptr, &sc1);

    if (FAILED(hr)) {
        if (result) *result = ngfx::Result::Failed;
        return nullptr;
    }

    IDXGISwapChain3* sc3 = nullptr;
    sc1->QueryInterface(IID_PPV_ARGS(&sc3));

    if (result) *result = ngfx::Result::Ok;
    return new DxPresentLayer(dxDev, sc3, fmt, desc->width, desc->height, count);
}

} // namespace d3d12

// ---------------------------------------------------------------------------
// DLL Export
// ---------------------------------------------------------------------------
extern "C" {
NGFX_EXPORT ngfx::Factory* CreateFactory(bool debug_layer_enable, ngfx_LogCallback log_call) {
    return new d3d12::DxFactory(debug_layer_enable, log_call);
}
}
