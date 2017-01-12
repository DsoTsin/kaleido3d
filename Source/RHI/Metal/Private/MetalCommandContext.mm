#include "Kaleido3D.h"
#include "MetalRHI.h"
#include "MetalEnums.h"

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

void CommandContext::BeginRendering()
{
    m_RenderEncoder = [m_CmdBuffer renderCommandEncoderWithDescriptor:m_RenderpassDesc];
}

void CommandContext::SetIndexBuffer(const rhi::IndexBufferView& IBView)
{
    
}

void CommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView)
{
    
}

void CommandContext::SetPipelineState(uint32 HashCode, rhi::PipelineStateObjectRef PipelineState)
{
    
}

void CommandContext::SetViewport(const rhi::ViewportDesc & vpDesc)
{
    MTLViewport viewport = { vpDesc.Left, vpDesc.Top, vpDesc.Width, vpDesc.Height, vpDesc.MinDepth, vpDesc.MaxDepth };
    [m_RenderEncoder setViewport:viewport];
}

void CommandContext::SetPrimitiveType(rhi::EPrimitiveType Type)
{
    m_CurPrimType = g_PrimitiveType[Type];
}

void CommandContext::DrawInstanced(rhi::DrawInstancedParam param)
{
    [m_RenderEncoder drawPrimitives:m_CurPrimType
                        vertexStart:param.StartVertexLocation vertexCount:param.VertexCountPerInstance
                        instanceCount:param.InstanceCount baseInstance:param.StartInstanceLocation];
}

void CommandContext::DrawIndexedInstanced(rhi::DrawIndexedInstancedParam param)
{
//    [m_RenderEncoder drawIndexedPrimitives:m_CurPrimType indexCount:param.IndexCountPerInstance indexType:(MTLIndexType) indexBuffer:(nonnull id<MTLBuffer>) indexBufferOffset:(NSUInteger)]
}

void CommandContext::EndRendering()
{
    [m_RenderEncoder endEncoding];
}

void CommandContext::Dispatch(uint32 x, uint32 y, uint32 z)
{
    [m_ComputeEncoder dispatchThreadgroups:MTLSizeMake(x, y, z) threadsPerThreadgroup:MTLSizeMake(x, y, z)];
}


NS_K3D_METAL_END
