//
//  MetalDevice.h
//  kaleido3d
//
//  Created by QinZhou on 15/2/11.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//
#pragma once
#ifndef kaleido3d_MetalDevice_h
#define kaleido3d_MetalDevice_h
#include "Common.h"
#include <KTL/Singleton.hpp>

#import <Metal/Metal.h>
#import <simd/simd.h>
#import <QuartzCore/CAMetalLayer.h>

NS_K3D_METAL_BEGIN

class Device : public Singleton<Device>
{
public:
    static id<MTLDevice> GetDevice();
    static void ReleaseObj(id object);
        
    Device();
    ~Device();
        //void Init();
        
    void Destroy();
        
private:
        
    CAMetalLayer *          m_MetalLayer;
    id <MTLDevice>          m_Device;
    id <MTLCommandQueue>    m_CommandQueue;
};

NS_K3D_METAL_END

#endif
