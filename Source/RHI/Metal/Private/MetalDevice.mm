//
//  MetalDevice.m
//  kaleido3d
//
//  Created by QinZhou on 15/2/11.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//
#include "MetalRHI.h"


NS_K3D_METAL_BEGIN

Device::Device()
{
    m_Device = MTLCreateSystemDefaultDevice();
    m_CommandQueue = [m_Device newCommandQueue];
    m_MetalLayer = [CAMetalLayer layer];
    m_MetalLayer.device = m_Device;
    m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    m_MetalLayer.framebufferOnly = YES;
}

Device::~Device() {
}

void Device::Destroy() {
    
}

id<MTLDevice> Device::GetDevice() {
    return Device::Get().m_Device;
}

NS_K3D_METAL_END