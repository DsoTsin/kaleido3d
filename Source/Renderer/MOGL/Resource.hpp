#pragma once

class Texture;
struct TextureDescriptor;

struct Resource {

    virtual Texture* newTextureWithDescriptor(TextureDescriptor *descriptor, int offset, int bytesPerRow) = 0;

    virtual void* contents() = 0;
};