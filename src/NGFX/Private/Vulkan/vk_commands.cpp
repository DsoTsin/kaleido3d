#include "vk_common.h"

namespace vulkan {

    ngfx::RenderEncoder* GpuCommandBuffer::newRenderEncoder(ngfx::Result * result)
    {
        return nullptr;
    }

    ngfx::ComputeEncoder * GpuCommandBuffer::newComputeEncoder(ngfx::Result * result)
    {
        return nullptr;
    }

    ngfx::Result GpuCommandBuffer::newBlitEncoder()
    {
        return ngfx::Result();
    }

    ngfx::Result GpuCommandBuffer::newParallelRenderEncoder()
    {
        return ngfx::Result();
    }

    ngfx::RaytraceEncoder * GpuCommandBuffer::newRaytraceEncoder(ngfx::Result * result)
    {
        return nullptr;
    }

    ngfx::Result GpuCommandBuffer::commit()
    {
        return ngfx::Result();
    }

}