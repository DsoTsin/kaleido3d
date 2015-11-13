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
public:
    
private:
    id<MTLDevice>   m_Device;
};

class Device : public rhi::IDevice
{
public:
    static id<MTLDevice> GetDevice();
    static void ReleaseObj(id object);
    
    Device();
    ~Device();
    
    Result                  Create(rhi::IDeviceAdapter *, bool withDebug);
    rhi::ICommandContext*	NewCommandContext(rhi::ECommandType);
    rhi::IGpuResource*		NewGpuResource(rhi::EGpuResourceType type);
    rhi::ISampler*			NewSampler(const rhi::SamplerState&);
    rhi::IPipelineState*	NewPipelineState();
    rhi::ISyncPointFence*	NewFence();

    void Destroy();
    
private:
    
    CAMetalLayer *          m_MetalLayer;
    id <MTLDevice>          m_Device;
    id <MTLCommandQueue>    m_CommandQueue;
};


class CommandContext : public rhi::ICommandContext
{
public:
    CommandContext();
    
private:
    id<MTLCommandBuffer> m_CmdBuffer;
    id<MTLCommandQueue> m_CmdQueue;
};

class ComputeContext : public CommandContext {
public:
    ComputeContext();
    
private:
    id<MTLComputeCommandEncoder> m_ComputeEncoder;
};

class GraphicsContext : public CommandContext {
public:
    GraphicsContext();
    
    
private:
    id<MTLRenderCommandEncoder> m_RenderEncoder;
};



NS_K3D_METAL_END

#endif /* CommandContext_hpp */
