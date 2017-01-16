#include "Public/MetalRHIResource.h"
#include "MetalRHI.h"
#include "MetalEnums.h"

NS_K3D_METAL_BEGIN

MTLTextureType RHIMTLTextureType(rhi::EGpuResourceType const & type)
{
    switch(type)
    {
    case rhi::EGT_Texture1D:
        return MTLTextureType1D;
    case rhi::EGT_Texture1DArray:
        return MTLTextureType1DArray;
    case rhi::EGT_Texture2D:
        return MTLTextureType2D;
    case rhi::EGT_Texture3D:
        return MTLTextureType3D;
    case rhi::EGT_Texture2DArray:
        return MTLTextureType2DArray;
    }
    return MTLTextureType2D;
}

MTLTextureUsage RHIMTLTexUsage(rhi::EGpuMemViewType const & viewType, rhi::EGpuResourceAccessFlag const & accessType)
{
    MTLTextureUsage usage = MTLTextureUsageUnknown;
    switch (viewType) {
        case rhi::EGVT_SRV:
            usage |= MTLTextureUsagePixelFormatView;
            break;
        case rhi::EGVT_RTV:
            usage |= MTLTextureUsageRenderTarget;
            break;
        default:
            break;
    }
    switch (accessType) {
        case rhi::EGRAF_Read:
            usage |= MTLTextureUsageShaderRead;
            break;
        case rhi::EGRAF_Write:
            usage |= MTLTextureUsageShaderWrite;
            break;
        case rhi::EGRAF_ReadAndWrite:
            usage |= (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite);
            break;
        default:
            break;
    }
    return usage;
}

MTLTextureDescriptor * RHIMTLTexDesc(rhi::ResourceDesc const & desc)
{
    MTLTextureDescriptor* texDesc = [MTLTextureDescriptor new];
    texDesc.width = desc.TextureDesc.Width;
    texDesc.height = desc.TextureDesc.Height;
    texDesc.depth = desc.TextureDesc.Depth;
    texDesc.pixelFormat = g_PixelFormat[desc.TextureDesc.Format];
    texDesc.mipmapLevelCount = desc.TextureDesc.MipLevels;
    texDesc.arrayLength = desc.TextureDesc.Layers;
    texDesc.textureType = RHIMTLTextureType(desc.Type);
    texDesc.usage = RHIMTLTexUsage(desc.ViewType, desc.Flag);
    texDesc.resourceOptions;
    texDesc.cpuCacheMode;
    texDesc.storageMode;
    return texDesc;
}

Texture::Texture(Device * device, rhi::ResourceDesc const & desc)
: m_Device(device)
, m_Desc(desc)
, m_TexDesc(nil)
{
    m_TexDesc = RHIMTLTexDesc(desc);
    m_Tex = [m_Device->GetDevice() newTextureWithDescriptor:m_TexDesc];
}

Texture::~Texture()
{
}

void * Texture::Map(uint64 start, uint64 size)
{
    return nullptr;
}

void Texture::UnMap()
{
    
}

uint64 Texture::GetResourceLocation() const
{
    return 0;
}

rhi::ResourceDesc Texture::GetResourceDesc() const
{
    return m_Desc;
}

uint64 Texture::GetResourceSize() const
{
    return 0;
}

rhi::SamplerCRef Texture::GetSampler() const
{
    return nullptr;
}

void Texture::BindSampler(rhi::SamplerRef)
{
    
}

void Texture::SetResourceView(rhi::ShaderResourceViewRef)
{
    
}

rhi::ShaderResourceViewRef
Texture::GetResourceView() const
{
    return nullptr;
}

NS_K3D_METAL_END
