#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "D3D12RootSignature.h"

NS_K3D_D3D12_BEGIN


PipelineState::PipelineState()
{
}

void PipelineState::SetShader(rhi::EShaderType, rhi::IShaderBytes*)
{

}

void PipelineState::SetRasterizerState(const rhi::RasterizerState& RState)
{

}

void PipelineState::SetBlendState(const rhi::BlendState& BState)
{

}

void PipelineState::SetDepthStencilState(const rhi::DepthStencilState& DState)
{

}

void PipelineState::SetSampler(rhi::ISampler* RHISampler)
{
	Sampler * sampler = reinterpret_cast<Sampler*>(RHISampler);
	
}

void PipelineState::SetVertexInputLayout(rhi::IVertexInputLayout *)
{
}

void PipelineState::Finalize()
{
}

void PipelineState::DestroyAll()
{

}

void PipelineState::SetRootSignature(const RootSignature& RootSig)
{

}

const RootSignature & PipelineState::GetRootSignature() const
{
	K3D_ASSERT(m_RootSignature != nullptr, "RootSignature is null");
	return *m_RootSignature;
}

NS_K3D_D3D12_END