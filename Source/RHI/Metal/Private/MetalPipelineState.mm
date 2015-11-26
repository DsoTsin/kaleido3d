#include "Kaleido3D.h"
#include "MetalRHI.h"

NS_K3D_METAL_BEGIN

PipelineState::PipelineState()
{

}

PipelineState::~PipelineState()
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

void GraphicsPSO::SetDepthStencilState(const rhi::DepthStencilState &)
{
    
}

void GraphicsPSO::SetRasterizerState(const rhi::RasterizerState &)
{
    
}

void GraphicsPSO::SetPrimitiveTopology(const rhi::EPrimitiveType)
{
    
}

void GraphicsPSO::SetVertexInputLayout(rhi::IVertexInputLayout *)
{
    
}

void GraphicsPSO::SetSampler(rhi::ISampler *)
{
    
}

NS_K3D_METAL_END