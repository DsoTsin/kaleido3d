//
//  MetalDevice.m
//  kaleido3d
//
//  Created by QinZhou on 15/2/11.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//
#include "Kaleido3D.h"
#include <Core/LogUtil.h>
#include "MetalRHI.h"
#include "MetalEnums.h"

#define __debugbreak __builtin_trap

namespace rhi
{
    PFNEnumAllDevice    EnumAllDeviceAdapter =
    [](IDeviceAdapter** & adapterList, uint32* count)
    {
        NSArray<id<MTLDevice>> * deviceList = MTLCopyAllDevices();
        *count = deviceList.count;
        adapterList = new IDeviceAdapter*[*count];
        for (uint32 i = 0; i<*count; i++)
        {
            id<MTLDevice> device = [deviceList objectAtIndex:i];
            adapterList[i] = new ::k3d::metal::DeviceAdapter(device);
            NSLog(@"%@", [device name]);
        }
    };
}


NS_K3D_METAL_BEGIN

Device::Device()
{
}

Device::~Device() {
}

void Device::Destroy() {
    
}

id<MTLDevice> Device::GetDevice() {
    return m_Device;
}

Device::Result
Device::Create(rhi::IDeviceAdapter *adapter, bool withDebug)
{
    K3D_ASSERT(adapter!=nullptr);
    m_Device = static_cast<DeviceAdapter*>(adapter)->m_Device;
    m_CommandQueue = [m_Device newCommandQueue];
    m_MetalLayer = [CAMetalLayer layer];
    m_MetalLayer.device = m_Device;
    m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    m_MetalLayer.framebufferOnly = YES;
    return Result::DeviceFound;
}

rhi::ICommandContext*
Device::NewCommandContext(rhi::ECommandType Type)
{
    return nullptr;
}

rhi::IGpuResource*
Device::NewGpuResource(rhi::EGpuResourceType Type)
{
    return nullptr;
}

rhi::IPipelineStateObject*
Device::NewPipelineState(rhi::EPipelineType Type)
{
    PipelineState * pipeline = nullptr;
    switch (Type)
    {
        case rhi::EPSO_Graphics:
            pipeline = new GraphicsPSO;
            break;
        case rhi::EPSO_Compute:
            pipeline = new ComputePSO;
            break;
        default:
            break;
    }
    if(pipeline)
    {
        pipeline->m_Device = m_Device;
    }
    return pipeline;
}

rhi::ISampler*
Device::NewSampler(const rhi::SamplerState&)
{
    return nullptr;
}

rhi::ISyncPointFence*
Device::NewFence()
{
    return nullptr;
}

NS_K3D_METAL_END