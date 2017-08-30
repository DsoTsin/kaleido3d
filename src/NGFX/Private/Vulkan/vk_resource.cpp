#include "vk_common.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace vulkan {
    GpuBuffer::~GpuBuffer()
    {
    }

    ngfx::BufferView* GpuBuffer::newView(ngfx::Result * result)
    {
        return nullptr;
    }

    GpuTexture::~GpuTexture()
    {
    }

    ngfx::TextureView* GpuTexture::newView(ngfx::Result * result)
    {
        return nullptr;
    }
}