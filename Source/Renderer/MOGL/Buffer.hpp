#pragma once

#include "Resource.hpp"

class Buffer : public Resource {
public:

    Texture* newTextureWithDescriptor(TextureDescriptor *descriptor, int offset, int bytesPerRow) override;

    void* contents() override;
};