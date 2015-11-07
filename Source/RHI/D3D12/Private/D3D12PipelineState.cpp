#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "D3D12RootSignature.h"
#include "D3D12Enums.h"

NS_K3D_D3D12_BEGIN


PipelineState::PipelineState()
	: m_IsComputeState(false)
	, m_GraphicsPSODesc{}
	, m_ComputePSODesc{}
{
}

void PipelineState::SetShader(rhi::EShaderType Type, rhi::IShaderBytes* RShader)
{
	ShaderBytes * Sb = static_cast<ShaderBytes*>(RShader);
	K3D_ASSERT(Sb != nullptr);
	switch (Type)
	{
	case rhi::ES_Vertex:
		m_GraphicsPSODesc.VS = Sb->AsBC();
		break;
	case rhi::ES_Fragment:
		m_GraphicsPSODesc.PS = Sb->AsBC();
		break;
	case rhi::ES_Compute:
		m_ComputePSODesc.CS = Sb->AsBC();
		m_IsComputeState = true;
		break;
	case rhi::ES_Hull:
		m_GraphicsPSODesc.HS = Sb->AsBC();
		break;
	case rhi::ES_Domain:
		m_GraphicsPSODesc.DS = Sb->AsBC();
		break;
	case rhi::ES_Geometry:
		m_GraphicsPSODesc.GS = Sb->AsBC();
		break;
	}
}

void PipelineState::SetRasterizerState(const rhi::RasterizerState& RState)
{
	RHIRasterizerDesc(m_GraphicsPSODesc.RasterizerState, RState);
}

void PipelineState::SetBlendState(const rhi::BlendState& BState)
{
	RHIBlendDesc(m_GraphicsPSODesc.BlendState, BState);
}

void PipelineState::SetDepthStencilState(const rhi::DepthStencilState& DState)
{
	RHIDepthStencilDesc(m_GraphicsPSODesc.DepthStencilState, DState);
}

void PipelineState::SetSampler(rhi::ISampler* RHISampler)
{
	Sampler * sampler = reinterpret_cast<Sampler*>(RHISampler);
	
}

void PipelineState::SetVertexInputLayout(rhi::IVertexInputLayout * RHILayout)
{
	VertexInputLayout * layout = static_cast<VertexInputLayout*>(RHILayout);
	m_GraphicsPSODesc.InputLayout = {layout->m_InputLayout.get(), layout->m_ElementCount};
}

void PipelineState::SetPrimitiveTopology(const rhi::EPrimitiveType TopologyType)
{
	switch(TopologyType) 
	{
	case rhi::Triangles:
	case rhi::TriangleStrip:
		m_GraphicsPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		break;
	case rhi::Points:
		m_GraphicsPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		break;
	case rhi::Lines:
		m_GraphicsPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		break;
	default:
		m_GraphicsPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		break;
	}
}

void PipelineState::Finalize()
{
	if (m_IsComputeState)
	{
		ThrowIfFailed(m_Device->CreateComputePipelineState(&m_ComputePSODesc, IID_PPV_ARGS(m_pPSO.GetAddressOf())));
	}
	else 
	{
		m_GraphicsPSODesc.SampleMask = UINT_MAX;
		m_GraphicsPSODesc.NumRenderTargets = 1;
		m_GraphicsPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_GraphicsPSODesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&m_GraphicsPSODesc, IID_PPV_ARGS(m_pPSO.GetAddressOf())));
	}
}

void PipelineState::SetDevice(Device * pDevice)
{
	K3D_ASSERT(pDevice != nullptr);
	m_Device = pDevice->Get();
}

void PipelineState::DestroyAll()
{

}

void PipelineState::SetRootSignature(const RootSignature& RootSig)
{
	if (!m_IsComputeState)
		m_GraphicsPSODesc.pRootSignature = RootSig.GetSignature();
	else
		m_ComputePSODesc.pRootSignature = RootSig.GetSignature();
}

const RootSignature & PipelineState::GetRootSignature() const
{
	K3D_ASSERT(m_RootSignature != nullptr, "RootSignature is null");
	return *m_RootSignature;
}

NS_K3D_D3D12_END