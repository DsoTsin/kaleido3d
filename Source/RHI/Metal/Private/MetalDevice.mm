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
#if K3DPLATFORM_OS_MAC
#include <Cocoa/Cocoa.h>
#else
#include <UIKit/UIWindow.h>
#endif

#import <QuartzCore/CAMetalLayer.h>

#define __debugbreak __builtin_trap

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
//    m_MetalLayer = [CAMetalLayer layer];
//    m_MetalLayer.device = m_Device;
//    m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
//    m_MetalLayer.framebufferOnly = YES;
    return Result::DeviceFound;
}

rhi::CommandContextRef
Device::NewCommandContext(rhi::ECommandType Type)
{
    return nullptr;
}

rhi::GpuResourceRef
Device::NewGpuResource(rhi::ResourceDesc const&)
{
    return nullptr;
}

rhi::ShaderResourceViewRef
Device::NewShaderResourceView(rhi::GpuResourceRef, const rhi::ResourceViewDesc &)
{
    return nullptr;
}

rhi::SamplerRef
Device::NewSampler(const rhi::SamplerState&)
{
    return nullptr;
}

rhi::PipelineStateObjectRef
Device::NewPipelineState(rhi::PipelineDesc const&,rhi::PipelineLayoutRef,rhi::EPipelineType)
{
    return nullptr;
}

rhi::PipelineLayoutRef
Device::NewPipelineLayout(const rhi::PipelineLayoutDesc &table)
{
    return nullptr;
}

rhi::SyncFenceRef
Device::NewFence()
{
    return nullptr;
}

rhi::IDescriptorPool*
Device::NewDescriptorPool()
{
    return nullptr;
}

rhi::RenderViewportRef
Device::NewRenderViewport(void * winHandle, rhi::GfxSetting & setting)
{
#if K3DPLATFORM_OS_MAC
    NSWindow* nWindow = (__bridge NSWindow*)winHandle;
    NSView* view = nWindow.contentView;
    NSRect rect = [view frame];
#else
    UIWindow* nWindow = (__bridge UIWindow*)winHandle;
    UIView* view = nWindow.subviews[0];
    CGRect rect = [view frame];
#endif
    setting.Width = rect.size.width;
    setting.Height = rect.size.height;
    CAMetalLayer* mtlayer = (CAMetalLayer*)view.layer;
    mtlayer.device = GetDevice();
    return k3d::MakeShared<RenderViewport>(mtlayer);
}

RenderViewport::RenderViewport(CAMetalLayer * mtlLayer)
: m_Layer(mtlLayer)
{
}

RenderViewport::~RenderViewport()
{
}

bool RenderViewport::InitViewport(void *windowHandle, rhi::IDevice *pDevice, rhi::GfxSetting & setting)
{
    m_Width = setting.Width;
    m_Height = setting.Height;
    [m_Layer nextDrawable];
    return true;
}

rhi::RenderTargetRef
Device::NewRenderTarget(rhi::RenderTargetLayout const&)
{
    return nullptr;
}

NS_K3D_METAL_END
