#pragma once

#include "Resource.hpp"

struct PixelFormat;

class Texture : public Resource {
public:

    Texture* newTextureWithDescriptor(TextureDescriptor *descriptor, int offset, int bytesPerRow) override;

    void* contents() override;

    Texture* newTextureViewWithPixelFormat(PixelFormat *format);

    Resource* rootResource();
};
