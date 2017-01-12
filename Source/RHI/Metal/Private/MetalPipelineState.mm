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

void PipelineState::SetLayout(rhi::PipelineLayoutRef)
{
    
}

void PipelineState::SetShader(rhi::EShaderType, rhi::ShaderBundle const&)
{
    
}

void PipelineState::SetRasterizerState(const rhi::RasterizerState&)
{
    
}

void PipelineState::SetBlendState(const rhi::BlendState&)
{
    
}

void PipelineState::SetDepthStencilState(const rhi::DepthStencilState&)
{
    
}

void PipelineState::SetPrimitiveTopology(const rhi::EPrimitiveType)
{
    
}

void PipelineState::SetVertexInputLayout(rhi::VertexDeclaration const*, uint32 Count)
{
    
}

void PipelineState::SetRenderTargetFormat(const rhi::RenderTargetFormat &)
{
    
}

void PipelineState::SetSampler(rhi::SamplerRef)
{
    
}

void PipelineState::Finalize()
{
    
}

NS_K3D_METAL_END
