//
//  CommandContext.hpp
//  Renderer_Metal
//
//  Created by QinZhou on 15/9/2.
//
//
#pragma once
#ifndef __CommandContext_h__
#define __CommandContext_h__

#include "../Common.h"
#include "IRHI.h"

NS_K3D_METAL_BEGIN

class DeviceAdapter : public rhi::IDeviceAdapter
{
    friend class Device;
public:
    DeviceAdapter(id<MTLDevice> device)
    : m_Device(device) {}
    
private:
    id<MTLDevice>   m_Device;
};

class Device : public rhi::IDevice
{
public:
    id<MTLDevice> GetDevice();
    static void ReleaseObj(id object);
    
    Device();
    ~Device();
    
    Result                  Create(rhi::IDeviceAdapter *, bool withDebug);
    rhi::ICommandContext*	NewCommandContext(rhi::ECommandType);
    rhi::IGpuResource*		NewGpuResource(rhi::EGpuResourceType type);
    rhi::ISampler*			NewSampler(const rhi::SamplerState&);
    rhi::IPipelineStateObject*	NewPipelineState(rhi::EPipelineType);
    rhi::ISyncPointFence*	NewFence();

    void Destroy();
    
private:
    
    CAMetalLayer *          m_MetalLayer;
    id <MTLDevice>          m_Device;
    id <MTLCommandQueue>    m_CommandQueue;
};

class PipelineState : public rhi::IPipelineStateObject
{
    friend class Device;
public:
    PipelineState();
    ~PipelineState();
    
    virtual void SetShader(rhi::EShaderType, rhi::IShaderBytes*) override;
    virtual void Finalize() override;
    
private:
    id<MTLDevice>   m_Device;
};

class ComputePSO : public PipelineState, public rhi::IComputePipelineState
{
public:
    
    rhi::EPipelineType GetType () override
    {
        return rhi::EPipelineType::EPSO_Compute;
    }
private:
    id<MTLComputePipelineState> m_PipelineState;
};

class GraphicsPSO : public PipelineState, public rhi::IGraphicsPipelineState
{
public:
    GraphicsPSO();
    ~GraphicsPSO();
    
    rhi::EPipelineType GetType () override
    {
        return rhi::EPipelineType::EPSO_Graphics;
    }
    
    void SetRasterizerState(const rhi::RasterizerState&) override;
    void SetBlendState(const rhi::BlendState&) override;
    void SetDepthStencilState(const rhi::DepthStencilState&) override;
    void SetPrimitiveTopology(const rhi::EPrimitiveType) override;
    void SetVertexInputLayout(rhi::IVertexInputLayout *) override;
    void SetSampler(rhi::ISampler*)override;
    
private:
    MTLRenderPipelineDescriptor* m_PipelineDesc;
    id<MTLRenderPipelineState> m_PipelineState;
    id<MTLDepthStencilState> m_DepthStencilState;
};

class CommandContext : public rhi::ICommandContext
{
public:
    CommandContext();
    
    virtual ~CommandContext() override;
    
    void Detach(rhi::IDevice *) override;
    void CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src) override;
    void Execute(bool Wait) override;
    void Reset() override;
    
protected:
    virtual id<MTLCommandEncoder> GetCmdEncoder() = 0;
    
private:
    id<MTLCommandBuffer> m_CmdBuffer;
    id<MTLCommandQueue> m_CmdQueue;
};

class ComputeContext : public rhi::IComputeCommand, public CommandContext
{
public:
    ComputeContext();
    ~ComputeContext();
    
    void Dispatch(uint32 X = 1, uint32 Y =1, uint32 Z = 1) override;
    
private:
    id<MTLComputeCommandEncoder> m_ComputeEncoder;
};

class GraphicsContext : public rhi::IGraphicsCommand, public CommandContext
{
public:
    GraphicsContext();
    ~GraphicsContext();
    
    void SetIndexBuffer(const rhi::IndexBufferView& IBView) override;
    void SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView) override;
    void SetPipelineState(uint32 HashCode, rhi::IPipelineStateObject*) override;
    void SetViewport(const rhi::Viewport &) override;
    void SetPrimitiveType(rhi::EPrimitiveType) override;
    void DrawInstanced(rhi::DrawInstanceParam) override;
    void DrawIndexedInstanced(rhi::DrawIndexedInstancedParam) override;
    
private:
    id<MTLRenderCommandEncoder> m_RenderEncoder;
};



NS_K3D_METAL_END

#endif /* CommandContext_hpp */
