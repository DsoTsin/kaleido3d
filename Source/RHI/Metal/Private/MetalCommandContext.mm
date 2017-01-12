#include "Kaleido3D.h"
#include "MetalRHI.h"

NS_K3D_METAL_BEGIN

CommandContext::CommandContext()
{
    
}

CommandContext::~CommandContext()
{
    
}

void CommandContext::Detach(rhi::IDevice *)
{
    
}

void CommandContext::CopyTexture(const rhi::TextureCopyLocation& Dest, const rhi::TextureCopyLocation& Src)
{
    
}

void CommandContext::CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src, rhi::CopyBufferRegion const & Region)
{
    
}

void CommandContext::Execute(bool Wait)
{
    
}

void CommandContext::Reset()
{
    
}

void CommandContext::SetIndexBuffer(const rhi::IndexBufferView& IBView)
{
    
}

void CommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView)
{
    
}

void CommandContext::SetPipelineState(uint32 HashCode, rhi::PipelineStateObjectRef)
{
    
}

void CommandContext::SetViewport(const rhi::ViewportDesc &)
{
    
}

void CommandContext::SetPrimitiveType(rhi::EPrimitiveType)
{
    
}

void CommandContext::DrawInstanced(rhi::DrawInstancedParam)
{
    
}

void CommandContext::DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)
{
    
}

void CommandContext::Dispatch(uint32 x, uint32 y, uint32 z)
{
    
}


NS_K3D_METAL_END
