#include "Kaleido3D.h"
#include "MetalRHI.h"
#include "Public/MetalRHIResource.h"

NS_K3D_METAL_BEGIN

rhi::GpuResourceRef
Device::NewGpuResource(rhi::ResourceDesc const& desc)
{
    switch (desc.Type) {
        case rhi::EGT_Buffer:
            return k3d::MakeShared<Buffer>(this, desc);
        case rhi::EGT_Texture2D:
            return k3d::MakeShared<Texture>(this, desc);
        default:
            break;
    }
    return nullptr;
}

NS_K3D_METAL_END
