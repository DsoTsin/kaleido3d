#pragma once

#include <Core/Image.h>
#import <Metal/MTLTexture.h>

namespace k3d
{
    class MetalImage : public Image
    {
    public:
        MetalImage();
        virtual ~MetalImage();
        
        bool Load(uint8 *dataPtr, uint32 length) override;
        bool IsCompressed() const override;
        bool IsCubeMap() const override;
        
        static id<MTLTexture> CreateTexture(MetalImage const & image);
        
    private:
        
        MTLTextureDescriptor* m_TexDesc;
    };
}