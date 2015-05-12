#include "Kaleido3D.h"

#include "Renderer.h"
#include "Device.h"
#include <d3dx12.h>

using namespace DirectX;

namespace k3d {

	using namespace d3d12;

	DirectXRenderer::~DirectXRenderer() {
	}

	void DirectXRenderer::Initialize()
	{
		Init();
		auto d3dDevice = Device::Get().GetD3DDevice();
		ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(m_BundleAllocator.GetInitReference())));

		/* Create Root Signature */
		{
			CD3DX12_DESCRIPTOR_RANGE range;
			CD3DX12_ROOT_PARAMETER parameter;

			range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

			PtrBlob pSignature;
			PtrBlob pError;
			ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetInitReference(), pError.GetInitReference()));
			ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetInitReference())));
		}

		{
			Enqueue([this, &d3dDevice]() {
				m_VS.Load("Test.vso");
				m_PS.Load("Test.pso");

				static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
				state.InputLayout = { inputLayout, _countof(inputLayout) };
				state.pRootSignature = m_RootSignature;
				state.VS = { m_VS.GetBlob()->GetBufferPointer(), m_VS.GetBlob()->GetBufferSize() };
				state.PS = { m_PS.GetBlob()->GetBufferPointer(), m_PS.GetBlob()->GetBufferSize() };
				state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				state.DepthStencilState.DepthEnable = FALSE;
				state.DepthStencilState.StencilEnable = FALSE;
				state.SampleMask = UINT_MAX;
				state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				state.NumRenderTargets = 1;
				state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				state.SampleDesc.Count = 1;

				ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(m_PipeLineState.GetInitReference())));

			});

			Enqueue([this, &d3dDevice]() {
			
			
			});
		}


	}

	void DirectXRenderer::PrepareFrame() {
		if ( !isInitialized ) {
			isInitialized = true;
		}
	}

	void DirectXRenderer::DrawOneFrame() {
		if (!isInitialized)
			return;
	}

	void DirectXRenderer::EndOneFrame() {
		this->SwapBuffers();
	}

	void DirectXRenderer::DrawMesh(IRenderMesh *) {

	}

	void DirectXRenderer::DrawMesh(IRenderMesh *, d3d12::Shader *)
	{
	}

	void DirectXRenderer::DrawMesh(IRenderMesh *, d3d12::Shader *, Matrix4f const & matrix)
	{

	}
	
	void DirectXRenderer::OnResize(int width, int height) {

	}

	void DirectXRenderer::SwapBuffers()
	{
	}

}