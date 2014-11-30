//
//  MetalDevice.m
//  kaleido3d
//
//  Created by QinZhou on 15/2/11.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//

#include "MetalDevice.h"

namespace k3d
{
    MetalDevice::MetalDevice()
    {
        m_Device = MTLCreateSystemDefaultDevice();
        m_CommandQueue = [m_Device newCommandQueue];
        m_MetalLayer = [CAMetalLayer layer];
        m_MetalLayer.device = m_Device;
        m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        m_MetalLayer.framebufferOnly = YES;
    }
    
    MetalDevice::~MetalDevice()
    {
        
    }
    
    void MetalDevice::Destroy()
    {
        
    }
    
    id<MTLDevice> MetalDevice::GetDevice()
    {
        return MetalDevice::Get().m_Device;
    }
    
}