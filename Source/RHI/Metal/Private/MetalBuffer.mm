#include "Public/MetalRHIResource.h"
#include "Private/MetalRHI.h"

NS_K3D_METAL_BEGIN

Buffer::Buffer(Device* device, rhi::ResourceDesc const & desc)
: m_Desc(desc)
, m_Device(device)
{
    m_Buf = [m_Device->GetDevice()
             newBufferWithLength:desc.Size
             options:MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeShared];
}

Buffer::~Buffer()
{
}

void * Buffer::Map(uint64 start, uint64 size)
{
    return [m_Buf contents];
}

void Buffer::UnMap()
{
#if K3DPLATFORM_OS_MAC
    [m_Buf didModifyRange:m_MapRange];
#endif
}

uint64 Buffer::GetResourceLocation() const
{
    return 0;
}

rhi::ResourceDesc Buffer::GetResourceDesc() const
{
    return m_Desc;
}

rhi::EResourceState Buffer::GetUsageState() const
{
    return rhi::ERS_Unknown;
}

rhi::EGpuResourceType Buffer::GetResourceType() const
{
    return rhi::EGT_Buffer;
}

uint64 Buffer::GetResourceSize() const
{
    return m_Buf.length;
}

NS_K3D_METAL_END
