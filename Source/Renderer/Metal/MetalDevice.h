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

#include <KTL/Singleton.hpp>

#import <Metal/Metal.h>
#import <simd/simd.h>
#import <QuartzCore/CAMetalLayer.h>

namespace k3d
{
    class MetalDevice : public Singleton<MetalDevice>
    {
    public:
        
        static id<MTLDevice> GetDevice();
        static void ReleaseObj(id object);
        
        MetalDevice();
        ~MetalDevice();
        //void Init();
        
        void Destroy();
        
    private:
        
        CAMetalLayer *          m_MetalLayer;
        id <MTLDevice>          m_Device;
        id <MTLCommandQueue>    m_CommandQueue;
    };
}

#endif
