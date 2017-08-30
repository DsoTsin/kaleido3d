#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "D3D12RootSignature.h"
#include "D3D12Enums.h"

NS_K3D_D3D12_BEGIN

PipelineLayout::PipelineLayout()
{

}

PipelineLayout::PipelineLayout(rhi::ShaderParamLayout const &ShaderParamLayout)
{
	CreateFromShaderLayout(ShaderParamLayout);
}

PipelineLayout::~PipelineLayout()
{

}

void PipelineLayout::Create(rhi::ShaderParamLayout const & ShaderParamLayout)
{
	CreateFromShaderLayout(ShaderParamLayout);
}

void PipelineLayout::Finalize(rhi::IDevice * pRHIDevice)
{
	Device* pDevice = static_cast<Device*>(pRHIDevice);
	K3D_ASSERT(pDevice != nullptr);
	m_RootSignature.Finalize(pDevice->Get(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void PipelineLayout::CreateFromShaderLayout(rhi::ShaderParamLayout const &)
{

}

PipelineState::PipelineState(Device::Ptr pDevice)
	: D3D12RHIDeviceChild(pDevice)
	, m_GraphicsPSODesc{}
	, m_ComputePSODesc{}
{
}

PipelineState::~PipelineState()
{}

void PipelineState::SetShader(rhi::EShaderType Type, ::k3d::IShaderCompilerOutput* RShader)
{
	K3D_ASSERT(RShader && RShader->Length()>0 );
	switch (Type)
	{
	case rhi::ES_Vertex:
		m_GraphicsPSODesc.VS = { RShader->Bytes(), RShader->Length() };
		break;
	case rhi::ES_Fragment:
		m_GraphicsPSODesc.PS = { RShader->Bytes(), RShader->Length() };
		break;
	case rhi::ES_Compute:
		m_ComputePSODesc.CS = { RShader->Bytes(), RShader->Length() };
		break;
	case rhi::ES_Hull:
		m_GraphicsPSODesc.HS = { RShader->Bytes(), RShader->Length() };
		break;
	case rhi::ES_Domain:
		m_GraphicsPSODesc.DS = { RShader->Bytes(), RShader->Length() };
		break;
	case rhi::ES_Geometry:
		m_GraphicsPSODesc.GS = { RShader->Bytes(), RShader->Length() };
		break;
	}
}

void PipelineState::DestroyAll()
{

}

void PipelineState::SetLayout(rhi::PipelineLayoutRef Layout)
{
	auto pLayout = StaticPointerCast<PipelineLayout>(Layout);
	SetRootSignature(pLayout->m_RootSignature);
}

void PipelineState::SetRootSignature(const RootSignature& RootSig)
{
	if (GetType() == rhi::EPipelineType::EPSO_Graphics)
		m_GraphicsPSODesc.pRootSignature = RootSig.GetSignature();
	else
		m_ComputePSODesc.pRootSignature = RootSig.GetSignature();
}

void PipelineState::Finalize()
{
	if (GetType() == rhi::EPipelineType::EPSO_Compute)
	{
		ThrowIfFailed(GetParentDeviceRef().Get()->CreateComputePipelineState(&m_ComputePSODesc, IID_PPV_ARGS(m_pPSO.GetAddressOf())));
	}
	else
	{
		/*
		m_GraphicsPSODesc.SampleMask = UINT_MAX;
		m_GraphicsPSODesc.NumRenderTargets = 1;
		m_GraphicsPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_GraphicsPSODesc.SampleDesc.Count = 1;
		*/
		ThrowIfFailed(GetParentDeviceRef().Get()->CreateGraphicsPipelineState(&m_GraphicsPSODesc, IID_PPV_ARGS(m_pPSO.GetAddressOf())));
	}
}

const RootSignature & PipelineState::GetRootSignature() const
{
	K3D_ASSERT(m_RootSignature != nullptr, "RootSignature is null");
	return *m_RootSignature;
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

void PipelineState::SetSampler(rhi::SamplerRef)
{
}

void PipelineState::SetVertexInputLayout(rhi::VertexDeclaration const *, uint32 Count)
{
	/*
	VertexInputLayout * layout = static_cast<VertexInputLayout*>(RHILayout);
	m_GraphicsPSODesc.InputLayout = {layout->m_InputLayout.get(), layout->m_ElementCount};*/
}

void PipelineState::SetPrimitiveTopology(const rhi::EPrimitiveType TopologyType)
{
	m_GraphicsPSODesc.PrimitiveTopologyType = g_PrimType[TopologyType];
}

void PipelineState::SetRenderTargetFormat(const rhi::RenderTargetFormat & RTFormat)
{
	DXGI_FORMAT * RTFormats = (DXGI_FORMAT *)calloc(RTFormat.NumRTs, sizeof(DXGI_FORMAT));
	for (uint32 i = 0; i < RTFormat.NumRTs; i++)
	{
		RTFormats[i] = g_DXGIFormatTable[RTFormat.RenderPixelFormats[i]];
	}
	SetRenderTargetFormats(RTFormat.NumRTs, RTFormats, g_DXGIFormatTable[RTFormat.DepthPixelFormat], RTFormat.MSAACount, 0);
}

void PipelineState::SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT * RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
{
	K3D_ASSERT(NumRTVs == 0 || RTVFormats != nullptr, "Null format array conflicts with non-zero length");
	for (UINT i = 0; i < NumRTVs; ++i)
		m_GraphicsPSODesc.RTVFormats[i] = RTVFormats[i];
	for (UINT i = NumRTVs; i < m_GraphicsPSODesc.NumRenderTargets; ++i)
		m_GraphicsPSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	m_GraphicsPSODesc.NumRenderTargets = NumRTVs;
	m_GraphicsPSODesc.DSVFormat = DSVFormat;
	m_GraphicsPSODesc.SampleDesc.Count = MsaaCount;
	m_GraphicsPSODesc.SampleDesc.Quality = MsaaQuality;
}

NS_K3D_D3D12_END