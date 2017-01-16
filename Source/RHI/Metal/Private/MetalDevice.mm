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

Device::Device(id<MTLDevice> pDevice) : m_Device(pDevice)
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
    return Result::DeviceFound;
}

rhi::CommandContextRef
Device::NewCommandContext(rhi::ECommandType Type)
{
    auto CmdBuf = [m_CommandQueue commandBuffer];
    return k3d::MakeShared<CommandContext>(Type, CmdBuf);
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

// The pixel format for a Metal layer must be bgra8Unorm, bgra8Unorm_srgb, or rgba16Float.
// The default value is bgra8Unorm. https://developer.apple.com/reference/quartzcore/cametallayer/1478155-pixelformat
MTLPixelFormat CorrectFormat(rhi::GfxSetting & setting)
{
    auto pf = g_PixelFormat[setting.ColorFormat];
    switch (pf) {
        case MTLPixelFormatRGBA16Float:
        case MTLPixelFormatBGRA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm:
            return pf;
        default:
            setting.ColorFormat = rhi::EPF_BGRA8Unorm;
            break;
    }
    return g_PixelFormat[setting.ColorFormat];
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
    mtlayer.device          = GetDevice();
    mtlayer.pixelFormat     = CorrectFormat(setting);
    mtlayer.framebufferOnly = YES;
    return k3d::MakeShared<RenderViewport>(mtlayer);
}

RenderViewport::RenderViewport(CAMetalLayer * mtlLayer)
: m_Layer(mtlLayer)
, m_CurrentDrawable(nil)
, m_RenderPassDescriptor(nil)
{
    m_RenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
}

RenderViewport::~RenderViewport()
{
}

bool RenderViewport::InitViewport(void *windowHandle, rhi::IDevice *pDevice, rhi::GfxSetting & setting)
{
    m_Width = setting.Width;
    m_Height = setting.Height;
    m_CurrentDrawable = [m_Layer nextDrawable];
    id<MTLTexture> texture = m_CurrentDrawable.texture;
    MTLRenderPassColorAttachmentDescriptor *colorAttachment = m_RenderPassDescriptor.colorAttachments[0];
    colorAttachment.texture = texture;
    
    // make sure to clear every frame for best performance
    colorAttachment.loadAction = MTLLoadActionClear;
    colorAttachment.clearColor = MTLClearColorMake(0.65f, 0.65f, 0.65f, 1.0f);
    
    // if sample count is greater than 1, render into using MSAA, then resolve into our color texture
//    if(_sampleCount > 1)
//    {
//        BOOL doUpdate =     ( _msaaTex.width       != texture.width  )
//        ||  ( _msaaTex.height      != texture.height )
//        ||  ( _msaaTex.sampleCount != _sampleCount   );
//        
//        if(!_msaaTex || (_msaaTex && doUpdate))
//        {
//            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: MTLPixelFormatBGRA8Unorm
//                                                                                            width: texture.width
//                                                                                           height: texture.height
//                                                                                        mipmapped: NO];
//            desc.textureType = MTLTextureType2DMultisample;
//            
//            // sample count was specified to the view by the renderer.
//            // this must match the sample count given to any pipeline state using this render pass descriptor
//            desc.sampleCount = _sampleCount;
//            
//            _msaaTex = [_device newTextureWithDescriptor: desc];
//        }
//        
//        // When multisampling, perform rendering to _msaaTex, then resolve
//        // to 'texture' at the end of the scene
//        colorAttachment.texture = _msaaTex;
//        colorAttachment.resolveTexture = texture;
//        
//        // set store action to resolve in this case
//        colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
//    }
//    else
    {
        // store only attachments that will be presented to the screen, as in this case
        colorAttachment.storeAction = MTLStoreActionStore;
    } // color0
    
    // Now create the depth and stencil attachments
    
    if(setting.HasDepth)
    {
//        BOOL doUpdate =     ( _depthTex.width       != texture.width  )
//        ||  ( _depthTex.height      != texture.height )
//        ||  ( _depthTex.sampleCount != _sampleCount   );
//        
//        if(!_depthTex || doUpdate)
//        {
            //  If we need a depth texture and don't have one, or if the depth texture we have is the wrong size
            //  Then allocate one of the proper size
            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: g_PixelFormat[setting.DepthStencilFormat]
                                                                                            width: texture.width
                                                                                           height: texture.height
                                                                                        mipmapped: NO];
            
            desc.textureType = /*(_sampleCount > 1) ? MTLTextureType2DMultisample :*/ MTLTextureType2D;
#if K3DPLATFORM_OS_MAC
            desc.resourceOptions = MTLResourceStorageModePrivate;
#endif
            //desc.sampleCount = _sampleCount;
            
            m_DepthTex = [m_Layer.device newTextureWithDescriptor: desc];
            
            MTLRenderPassDepthAttachmentDescriptor *depthAttachment = m_RenderPassDescriptor.depthAttachment;
            depthAttachment.texture = m_DepthTex;
            depthAttachment.loadAction = MTLLoadActionClear;
            depthAttachment.storeAction = MTLStoreActionDontCare;
            depthAttachment.clearDepth = 1.0;
//        }
    } // depth
    
//    if(_stencilPixelFormat != MTLPixelFormatInvalid)
//    {
//        BOOL doUpdate  =    ( _stencilTex.width       != texture.width  )
//        ||  ( _stencilTex.height      != texture.height )
//        ||  ( _stencilTex.sampleCount != _sampleCount   );
//        
//        if(!_stencilTex || doUpdate)
//        {
//            //  If we need a stencil texture and don't have one, or if the depth texture we have is the wrong size
//            //  Then allocate one of the proper size
//            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: _stencilPixelFormat
//                                                                                            width: texture.width
//                                                                                           height: texture.height
//                                                                                        mipmapped: NO];
//            
//            desc.textureType = (_sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
//            desc.sampleCount = _sampleCount;
//            
//            _stencilTex = [_device newTextureWithDescriptor: desc];
//            
//            MTLRenderPassStencilAttachmentDescriptor* stencilAttachment = m_RenderPassDescriptor.stencilAttachment;
//            stencilAttachment.texture = _stencilTex;
//            stencilAttachment.loadAction = MTLLoadActionClear;
//            stencilAttachment.storeAction = MTLStoreActionDontCare;
//            stencilAttachment.clearStencil = 0;
//        }
//    } //stencil
    
    return true;
}

void RenderViewport::PrepareNextFrame()
{
    m_CurrentDrawable = [m_Layer nextDrawable];
}

rhi::RenderTargetRef
Device::NewRenderTarget(rhi::RenderTargetLayout const&)
{
    return nullptr;
}

NS_K3D_METAL_END
