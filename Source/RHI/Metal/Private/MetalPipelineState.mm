#include "Kaleido3D.h"
#include "MetalRHI.h"
#include <Core/LogUtil.h>

NS_K3D_METAL_BEGIN

PipelineState::PipelineState()
{

}

PipelineState::~PipelineState()
{
    
}

void PipelineState::SetLayout(rhi::IPipelineLayout *)
{
    
}

void PipelineState::SetShader(rhi::EShaderType, rhi::IShaderBytes *)
{
    
}

void PipelineState::Finalize()
{
    
}

GraphicsPSO::GraphicsPSO()
: PipelineState()
, m_PipelineDesc(nullptr)
{
    m_PipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
}

GraphicsPSO::~GraphicsPSO()
{
    [m_PipelineDesc release];
}


void GraphicsPSO::SetBlendState(const rhi::BlendState & BlendState)
{
    
}

void GraphicsPSO::SetDepthStencilState(const rhi::DepthStencilState & DepthStencilState)
{
    
}

void GraphicsPSO::SetRasterizerState(const rhi::RasterizerState & RasterState)
{
    
}

void GraphicsPSO::SetPrimitiveTopology(const rhi::EPrimitiveType PrimType)
{
    
}

void GraphicsPSO::SetVertexInputLayout(rhi::VertexDeclaration * VertDec, uint32 Count)
{
    K3D_ASSERT(VertDec && Count > 0);
}

void GraphicsPSO::SetRenderTargetFormat(const rhi::RenderTargetFormat & RTF)
{
    
}

void GraphicsPSO::SetSampler(rhi::ISampler * iSampler)
{
    
}

NS_K3D_METAL_END