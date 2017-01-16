#pragma once
#ifndef __MetalRHIResource_h__
#define __MetalRHIResource_h__

#include "../Common.h"
#include <Interface/IRHI.h>

NS_K3D_METAL_BEGIN

class Device;

class Buffer : public rhi::IGpuResource
{
public:
    Buffer(Device* device, rhi::ResourceDesc const & desc);
    ~Buffer();
    
    void *                      Map(uint64 start, uint64 size) override;
    void                        UnMap() override;
    
    uint64                      GetResourceLocation() const override;
    rhi::ResourceDesc           GetResourceDesc() const override;
    rhi::EResourceState         GetUsageState() const override;
    rhi::EGpuResourceType       GetResourceType() const	override;
    uint64                      GetResourceSize() const override;
    
private:
    NSRange             m_MapRange;
    id<MTLBuffer>       m_Buf;
    
    rhi::ResourceDesc   m_Desc;
    Device*             m_Device;
};

class Texture : public rhi::ITexture
{
public:
    Texture(Device* device, rhi::ResourceDesc const & desc);
    ~Texture();
    
    void *                      Map(uint64 start, uint64 size) override;
    void                        UnMap() override;
    
    uint64                      GetResourceLocation() const override;
    rhi::ResourceDesc           GetResourceDesc() const override;
    rhi::EResourceState         GetUsageState() const override { return rhi::ERS_Unknown; }
    rhi::EGpuResourceType       GetResourceType() const	override { return rhi::ResourceTypeNum; }
    uint64                      GetResourceSize() const override;
    
    rhi::SamplerCRef			GetSampler() const override;
    void                        BindSampler(rhi::SamplerRef) override;
    void                        SetResourceView(rhi::ShaderResourceViewRef) override;
    rhi::ShaderResourceViewRef 	GetResourceView() const override;
    
private:
    id<MTLTexture>          m_Tex;
    MTLTextureDescriptor*   m_TexDesc;
    rhi::ResourceDesc       m_Desc;
    MTLStorageMode          storageMode;
    
    MTLTextureUsage         usage;
    Device*                 m_Device;
};


NS_K3D_METAL_END

#endif
