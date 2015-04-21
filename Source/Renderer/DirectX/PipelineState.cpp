#include "Kaleido3D.h"

#include "PipelineState.h"

namespace k3d
{
  namespace d3d12
  {
	  PipelineStateObjectDescription PipelineStateObjectDescription::Simple(
		  const D3D12_INPUT_LAYOUT_DESC & inputLayout,
		  const RootSignature & rootSig,
		  const Shader & vs, const Shader & ps)
	  {
		  ID3DBlob* vsBlob = vs.GetBlob();
		  ID3DBlob* psBlob = ps.GetBlob();
		  PipelineStateObjectDescription psoDesc;
		  ZeroMemory(&psoDesc, sizeof(psoDesc));
		  psoDesc.InputLayout = inputLayout;
		  psoDesc.pRootSignature = rootSig.Get();
		  psoDesc.VS = { reinterpret_cast<BYTE*>(vsBlob->GetBufferPointer()), vsBlob->GetBufferSize() };
		  psoDesc.PS = { reinterpret_cast<BYTE*>(psBlob->GetBufferPointer()), psBlob->GetBufferSize() };

		  psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		  psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		  psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
		  psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		  psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		  psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		  psoDesc.RasterizerState.DepthClipEnable = TRUE;
		  psoDesc.RasterizerState.MultisampleEnable = FALSE;
		  psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		  psoDesc.RasterizerState.ForcedSampleCount = 0;
		  psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		  psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		  psoDesc.BlendState.IndependentBlendEnable = FALSE;
		  const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		  {
			  FALSE,FALSE,
			  D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			  D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			  D3D12_LOGIC_OP_NOOP,
			  D3D12_COLOR_WRITE_ENABLE_ALL,
		  };
		  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		  {
			  psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
		  }

		  psoDesc.DepthStencilState.DepthEnable = FALSE;
		  psoDesc.DepthStencilState.StencilEnable = FALSE;
		  psoDesc.SampleMask = UINT_MAX;
		  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		  psoDesc.NumRenderTargets = 1;
		  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		  psoDesc.SampleDesc.Count = 1;

		  //psoDesc.RasterizerState.DepthClipEnable = false;

		  return psoDesc;
	  }

	  void PipelineStateObject::Create(ID3D12Device * device, const PipelineStateObjectDescription & desc)
	  {
		  ThrowIfFailed(
			  device->CreateGraphicsPipelineState(
				  &desc,
				  IID_PPV_ARGS(mPSO.GetInitReference()))
			);
	  }
  }
}