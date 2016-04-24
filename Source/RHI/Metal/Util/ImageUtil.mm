#include "Kaleido3D.h"
#include "ImageUtil.h"

namespace k3d
{
    MetalImage::MetalImage()
    : m_TexDesc(nil)
    {
        
    }
    
    MetalImage::~MetalImage()
    {
        
    }
    
    bool MetalImage::IsCompressed() const
    {
        return false;
    }
    
    bool MetalImage::IsCubeMap() const
    {
        return false;
    }
    
    bool MetalImage::Load(uint8 *dataPtr, uint32 length)
    {
        
        return false;
    }
    
    id<MTLTexture> MetalImage::CreateTexture(const MetalImage &image) {
        id<MTLTexture> texture /*= [Device::GetDevice() newTextureWithDescriptor:image.m_TexDesc]*/;
        return texture;
    }
    
}