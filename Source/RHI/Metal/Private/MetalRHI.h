//
//  CommandContext.hpp
//  RHI_Metal
//
//  Created by QinZhou on 15/9/2.
//
//
#pragma once
#ifndef __CommandContext_h__
#define __CommandContext_h__

#include "../Common.h"
#include <Interface/IRHI.h>

NS_K3D_METAL_BEGIN

class Device;
using SpDevice = SharedPtr<Device>;

class Factory : public k3d::IFactory
{
public:
    void EnumDevices(k3d::DynArray<k3d::DeviceRef>& Devices) override;
    k3d::SwapChainRef CreateSwapchain(k3d::CommandQueueRef pCommandQueue,
                                      void* nPtr,
                                      k3d::SwapChainDesc&) override;
};

#pragma mark Device

class Device : public k3d::IDevice, public EnableSharedFromThis<Device>
{
public:
    id<MTLDevice> GetDevice();
    static void ReleaseObj(id object);
    
    Device();
    ~Device() override;
    // IObject::Release
    void Release() override;

    k3d::GpuResourceRef         CreateResource(k3d::ResourceDesc const&)override;
    k3d::ShaderResourceViewRef	CreateShaderResourceView(k3d::GpuResourceRef, k3d::SRVDesc const&)override;
    k3d::UnorderedAccessViewRef CreateUnorderedAccessView(const k3d::GpuResourceRef&, k3d::UAVDesc const&) override;
    k3d::SamplerRef             CreateSampler(const k3d::SamplerState&)override;
    k3d::PipelineLayoutRef		CreatePipelineLayout(k3d::PipelineLayoutDesc const & table) override;
    k3d::SyncFenceRef           CreateFence()override;
    k3d::RenderPassRef          CreateRenderPass(k3d::RenderPassDesc const&) override;
    
    k3d::PipelineStateRef       CreateRenderPipelineState(k3d::RenderPipelineStateDesc const&,
                                                          k3d::PipelineLayoutRef,
                                                          k3d::RenderPassRef) override;
    k3d::PipelineStateRef       CreateComputePipelineState(k3d::ComputePipelineStateDesc const&,
                                                           k3d::PipelineLayoutRef) override;
    k3d::CommandQueueRef        CreateCommandQueue(k3d::ECommandType const&) override;
    void                        WaitIdle() override;
    
    friend struct DeviceChild;
private:
    Device(id<MTLDevice> device);
    
    id <MTLDevice>          m_Device;
    id <MTLCommandQueue>    m_CommandQueue;
    MTLFeatureSet           m_FeatureSet;
};


#pragma mark Command

class CommandQueue : public k3d::ICommandQueue
{
    
};

class CommandBuffer : public k3d::ICommandBuffer
{
    
};

class CommandEncoder : public k3d::ICommandEncoder
{
    
};

template <typename RHIInterface>
struct MTLObj
{
};

template<>
struct MTLObj<k3d::IRenderPipelineState>
{
    typedef id<MTLRenderPipelineState> RawType;
    typedef MTLRenderPipelineDescriptor Desc;
};

template<>
struct MTLObj<k3d::IComputePipelineState>
{
    typedef id<MTLComputePipelineState> RawType;
    typedef MTLComputePipelineDescriptor Desc;
};

struct DeviceChild
{
    DeviceChild(SpDevice pDevice)
    : Device(pDevice)
    {
    }
    
    id<MTLDevice> NativeDevice() const
    {
        return Device->m_Device;
    }
    
    id<MTLCommandQueue> NativeQueue() const
    {
        return Device->m_CommandQueue;
    }
    
    SpDevice Device;
};

#pragma mark Pipeline

template <typename T>
struct TPipelineType
{
    static const EPipelineType Type = EPSO_Graphics;
};

template <>
struct TPipelineType<k3d::IRenderPipelineState>
{
    static const EPipelineType Type = EPSO_Graphics;
};

template <>
struct TPipelineType<k3d::IComputePipelineState>
{
    static const EPipelineType Type = EPSO_Compute;
};

template <typename T>
class TPipeline : public T, public DeviceChild
{
    friend class Device;
protected:
    typename MTLObj<T>::RawType m_NativeObj;
    typename MTLObj<T>::Desc* m_NativeDesc;
    
public:
    
    explicit TPipeline(SpDevice pDevice) : DeviceChild(pDevice)
    {}
    
    virtual ~TPipeline() override
    {}
    
    EPipelineType GetType() const override
    { return TPipelineType<T>::Type; };
    
    //virtual void Rebuild() = 0;
};

class RenderPipeline : public TPipeline<k3d::IRenderPipelineState>
{
    using Super = TPipeline<k3d::IRenderPipelineState>;
public:
    
    RenderPipeline(SpDevice pDevice, k3d::RenderPipelineStateDesc const & desc);
    ~RenderPipeline() override;

    void SetRasterizerState(const k3d::RasterizerState&) override;
    void SetBlendState(const k3d::BlendState&) override;
    void SetDepthStencilState(const k3d::DepthStencilState&) override;
    void SetPrimitiveTopology(const k3d::EPrimitiveType) override;
    void SetRenderTargetFormat(const k3d::RenderTargetFormat &) override;
    void SetSampler(k3d::SamplerRef)override;

    void Rebuild() override;
    
private:
    void InitPSO(k3d::RenderPipelineStateDesc const & desc);
    void AssignShader(k3d::ShaderBundle const& shaderBundle);
    
    MTLDepthStencilDescriptor*      m_DepthStencilDesc;
    id<MTLDepthStencilState>        m_DepthStencilState;
    MTLCullMode                     m_CullMode;
    float                           m_DepthBias;
    float                           m_DepthBiasClamp;
    
    MTLPrimitiveType                m_PrimitiveType;
    
    MTLRenderPipelineReflection*    m_RenderReflection;
    //MTLComputePipelineReflection*   m_ComputeReflection;
};

class BindingGroup : public k3d::IBindingGroup
{
public:
    BindingGroup()
    {}
    
    void Update(uint32 bindSet, k3d::GpuResourceRef res) override;
    
private:
    
};

class PipelineLayout : public k3d::IPipelineLayout
{
public:
    PipelineLayout(k3d::PipelineLayoutDesc const&);
    ~PipelineLayout() override;
    
    k3d::BindingGroupRef ObtainBindingGroup() override;

private:
    
};

#pragma mark Resource

class Buffer : public k3d::IGpuResource
{
public:
    Buffer(Device* device, k3d::ResourceDesc const & desc);
    ~Buffer();
    
    void *                      Map(uint64 start, uint64 size) override;
    void                        UnMap() override;
    
    uint64                      GetLocation() const override;
    k3d::ResourceDesc           GetDesc() const override;
    k3d::EResourceState         GetState() const override;
    //    k3d::EGpuResourceType       GetType() const	override;
    uint64                      GetSize() const override;
    
private:
    NSRange             m_MapRange;
    id<MTLBuffer>       m_Buf;
    
    k3d::ResourceDesc   m_Desc;
    Device*             m_Device;
};

class Texture : public k3d::ITexture
{
public:
    Texture(Device* device, k3d::ResourceDesc const & desc);
    ~Texture();
    
    void *                      Map(uint64 start, uint64 size) override;
    void                        UnMap() override;
    
    uint64                      GetLocation() const override;
    k3d::ResourceDesc           GetDesc() const override;
    k3d::EResourceState         GetState() const override { return k3d::ERS_Unknown; }
    //    k3d::EGpuResourceType       GetResourceType() const	override { return k3d::ResourceTypeNum; }
    uint64                      GetSize() const override;
    
    k3d::SamplerCRef			GetSampler() const override;
    void                        BindSampler(k3d::SamplerRef) override;
    void                        SetResourceView(k3d::ShaderResourceViewRef) override;
    k3d::ShaderResourceViewRef 	GetResourceView() const override;
    
private:
    id<MTLTexture>          m_Tex;
    MTLTextureDescriptor*   m_TexDesc;
    k3d::ResourceDesc       m_Desc;
    MTLStorageMode          storageMode;
    
    MTLTextureUsage         usage;
    Device*                 m_Device;
};
#if 0
class CommandContext : public k3d::ICommandContext
{
public:
    CommandContext(k3d::ECommandType const & cmdType, id<MTLCommandBuffer> cmdBuf);
    
    ~CommandContext() override;
    
    void Detach(k3d::IDevice *) override;
    
    void CopyTexture(const k3d::TextureCopyLocation& Dest, const k3d::TextureCopyLocation& Src) override;
    void CopyBuffer(k3d::IGpuResource& Dest, k3d::IGpuResource& Src, k3d::CopyBufferRegion const & Region) override;
    void TransitionResourceBarrier(k3d::GpuResourceRef resource, /*EPipelineStage stage,*/ k3d::EResourceState dstState) override {}
    void Execute(bool Wait) override;
    void Reset() override;
    
    void Begin() override;
    
    void ClearColorBuffer(k3d::GpuResourceRef, kMath::Vec4f const&) override {}
    void ClearDepthBuffer(k3d::IDepthBuffer*) override {}
    
    void BeginRendering() override;
    void SetRenderTargets(uint32 NumColorBuffer, k3d::IColorBuffer*, k3d::IDepthBuffer*, bool ReadOnlyDepth = false) override {}
    void SetRenderTarget(k3d::RenderTargetRef) override;
    void SetPipelineState(uint32 HashCode, k3d::PipelineStateObjectRef) override;
    void SetPipelineLayout(k3d::PipelineLayoutRef) override {}
    void SetScissorRects(uint32, const k3d::Rect*) override {}
    void SetViewport(const k3d::ViewportDesc &) override;
    void SetPrimitiveType(k3d::EPrimitiveType) override;
    void SetIndexBuffer(const k3d::IndexBufferView& IBView) override;
    void SetVertexBuffer(uint32 Slot, const k3d::VertexBufferView& VBView) override;
    void DrawInstanced(k3d::DrawInstancedParam) override;
    void DrawIndexedInstanced(k3d::DrawIndexedInstancedParam) override;
    void EndRendering() override;
    void PresentInViewport(k3d::RenderViewportRef rvp) override;
    
    void Dispatch(uint32 X = 1, uint32 Y =1, uint32 Z = 1) override;
    
    void ExecuteBundle(k3d::ICommandContext*) override {}
    
    void End() override;
    
protected:
    
private:
    MTLRenderPassDescriptor*        m_RenderpassDesc;
    k3d::ECommandType               m_CommandType;
    MTLPrimitiveType                m_CurPrimType;
    id<MTLComputeCommandEncoder>    m_ComputeEncoder;
    id<MTLRenderCommandEncoder>     m_RenderEncoder;
    id <MTLParallelRenderCommandEncoder> m_ParallelRenderEncoder;
    id<MTLCommandBuffer>            m_CmdBuffer;
    
    id<MTLBuffer>                   m_TmpIndexBuffer;
};
class RenderViewport : public k3d::IRenderViewport
{
public:
    RenderViewport(CAMetalLayer * mtlLayer = nil);
    ~RenderViewport() override;
    
    bool                    InitViewport(void *windowHandle, k3d::IDevice * pDevice, k3d::GfxSetting &) override;
    void                    PrepareNextFrame() override;
    bool                    Present(bool vSync) override { return false; }
    
    k3d::RenderTargetRef    GetRenderTarget(uint32 index) override { return nullptr; }
    
    k3d::RenderTargetRef    GetCurrentBackRenderTarget() override;
    uint32                  GetSwapChainCount()override { return 0; }
    uint32                  GetSwapChainIndex()override { return 0; }
    
    uint32                  GetWidth() const override { return m_Width; }
    uint32                  GetHeight() const override { return m_Height; }
    
    friend class            Device;
    friend class            CommandContext;
private:
    
    CAMetalLayer *          m_Layer;
    id<CAMetalDrawable>     m_CurrentDrawable;
    MTLRenderPassDescriptor*m_RenderPassDescriptor;
    id<MTLTexture>          m_DepthTex;
    uint32                  m_Width;
    uint32                  m_Height;
};
#endif

class RenderTarget : public k3d::IRenderTarget
{
public:
    RenderTarget() {}
    RenderTarget(MTLRenderPassDescriptor* rpd, id<MTLTexture> color);
    ~RenderTarget() override;
    
    void                SetClearColor(kMath::Vec4f clrColor) override;
    void                SetClearDepthStencil(float depth, uint32 stencil) override;
    k3d::GpuResourceRef	GetBackBuffer() override;
    
    friend class        CommandContext;
private:
    MTLRenderPassDescriptor*m_RenderPassDescriptor = nil;
    id<MTLTexture>          m_ColorTexture;
};

class Sampler : public k3d::ISampler
{
public:
    explicit Sampler(k3d::SamplerState const & state);
    ~Sampler() override;
    
    k3d::SamplerState       GetSamplerDesc() const override;
    
private:
    id<MTLSamplerState>    m_SampleState;
};

NS_K3D_METAL_END

#endif /* CommandContext_hpp */
