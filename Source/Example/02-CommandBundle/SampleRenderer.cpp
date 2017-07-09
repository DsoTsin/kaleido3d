#include "Kaleido3D.h"
#include "CubeMesh.h"
#include "SampleRenderer.h"
#include <Renderer/DirectX/DeviceManager.h>

using namespace DirectX;

namespace k3d {

	namespace d3d12 {

		std::shared_ptr<DeviceManager>	gD3DDevice = std::make_shared<DeviceManager>();

		SampleRenderer::~SampleRenderer() {
			delete m_CubeMesh;
		}

		void SampleRenderer::Initialize()
		{
			m_RenderQueue = new ::Dispatch::WorkQueue("RenderQueue", ::Concurrency::ThreadPriority::High);
			m_RenderQueue->Loop();
			isInitialized = false;

			float aspectRatio = 1900.f/700;
			float fovAngleY = 70.0f * XM_PI / 180.0f;

			if (aspectRatio < 1.0f)
			{
				fovAngleY *= 2.0f;
			}

			XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);
			XMStoreFloat4x4(&m_MVP.projection, XMMatrixTranspose(perspectiveMatrix));

			static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
			static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
			static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
			XMStoreFloat4x4(&m_MVP.model, XMMatrixTranspose(XMMatrixRotationY(1.57)));
			XMStoreFloat4x4(&m_MVP.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

			auto d3dDevice = gD3DDevice->GetD3DDevice();
			ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(m_BundleAllocator.GetAddressOf())));

			CD3DX12_DESCRIPTOR_RANGE range;
			range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			CD3DX12_ROOT_PARAMETER parameter;
			parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

			CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
			descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

			PtrBlob pSignature, pError;
			ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
			ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
			
			m_RenderQueue->Queue(::Dispatch::Bind([this, d3dDevice]() {
				m_VS.Load(L"/Data/Test/Test.vso");
				m_PS.Load(L"/Data/Test/Test.pso");

				static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};

				D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
				state.InputLayout = { inputLayout, _countof(inputLayout) };
				state.pRootSignature = m_RootSignature.Get();
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

				ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(m_PipeLineState.GetAddressOf())));

				Log::Out("Renderer", ::Concurrency::Thread::GetCurrentThreadName());

			}));

			m_RenderQueue->Queue(::Dispatch::Bind([this, d3dDevice]() {
				ThrowIfFailed(d3dDevice->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, gD3DDevice->GetCommandAllocator(), m_PipeLineState.Get(), IID_PPV_ARGS(m_CmdList.GetAddressOf())));
				ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_BundleAllocator.Get(), m_PipeLineState.Get(), IID_PPV_ARGS(m_BundleCmdList.GetAddressOf())));

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.NumDescriptors = DeviceManager::GetFrameCount();
				heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_CBVHeap.GetAddressOf())));
				m_CBVHeap->SetName(KT("Constant Buffer Heap"));
				
				m_CubeMesh = new CubeMesh(d3dDevice, m_CmdList);

				m_CBO = new UniformBuffer<ModelViewProjectionConstantBuffer>("ModeViewMatrix", d3dDevice, 1U);
				m_CBO->CreateOnHeap(m_CBVHeap, d3dDevice);

				m_ConstantBuffer = m_CBO->Map();
				ZeroMemory(m_ConstantBuffer, DeviceManager::GetFrameCount() * m_CBO->sAlignedConstantBufferSize);

				ThrowIfFailed(m_CmdList->Close());
				ID3D12CommandList* ppCommandLists[] = { m_CmdList.Get() };
				gD3DDevice->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				gD3DDevice->WaitForGPU();

				m_BundleCmdList->SetGraphicsRootSignature(m_RootSignature.Get());

				PtrBlob blob;
				ThrowIfFailed(m_PipeLineState->GetCachedBlob(blob.GetAddressOf()));
				uint32 sizeBlob = blob->GetBufferSize();
				ID3D12DescriptorHeap* ppDescHeap[] = { m_CBVHeap.Get() };
				m_BundleCmdList->SetDescriptorHeaps(1, ppDescHeap);
				m_BundleCmdList->SetGraphicsRootDescriptorTable(0, m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
				m_CubeMesh->Render(m_BundleCmdList);
				ThrowIfFailed(m_BundleCmdList->Close());

				isInitialized = true; 
			}));
		}

		void SampleRenderer::PrepareFrame() {
			if (!isInitialized) {
				isInitialized = true;
			}
		}

		void SampleRenderer::DrawOneFrame() {
			if (!isInitialized)
				return;

			XMStoreFloat4x4(&m_MVP.model, XMMatrixTranspose(XMMatrixRotationY(1.57)));
			UINT8* destination = (UINT8*)m_ConstantBuffer + (gD3DDevice->GetCurrentFrameIndex() * m_CBO->sAlignedConstantBufferSize);
			memcpy(destination, &m_MVP, sizeof(m_MVP));

			ThrowIfFailed(gD3DDevice->GetCommandAllocator()->Reset());
			ThrowIfFailed(m_CmdList->Reset(gD3DDevice->GetCommandAllocator(), m_PipeLineState.Get()));

			m_CmdList->BeginEvent(1, "Draw the cube", strlen("Draw the cube"));
			{
				m_CmdList->SetGraphicsRootSignature(m_RootSignature.Get());
				ID3D12DescriptorHeap* ppHeaps[] = { m_CBVHeap.Get() };
				m_CmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

				m_CmdList->RSSetViewports(1, &(gD3DDevice->GetScreenViewport()));
				m_CmdList->RSSetScissorRects(1, &(gD3DDevice->GetScissorRect()));

				CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(gD3DDevice->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				m_CmdList->ResourceBarrier(1, &renderTargetResourceBarrier);
				
				m_CmdList->ClearRenderTargetView(gD3DDevice->GetRenderTargetView(), DirectX::Colors::CornflowerBlue, 0, nullptr);
				m_CmdList->OMSetRenderTargets(1, &(gD3DDevice->GetRenderTargetView()), false, nullptr);

				m_CmdList->ExecuteBundle(m_BundleCmdList.Get());

				m_CmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gD3DDevice->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
			}
			m_CmdList->EndEvent();

			ThrowIfFailed(m_CmdList->Close());
			ID3D12CommandList* ppCommandLists[] = { m_CmdList.Get() };
			gD3DDevice->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		}

		void SampleRenderer::EndOneFrame() {
			this->SwapBuffers();
		}

		void SampleRenderer::Render(d3d12::Renderable * r)
		{
			DrawOneFrame();
		}

		void SampleRenderer::OnResize(int width, int height) {

		}

		void SampleRenderer::SwapBuffers()
		{
		}
	}

}

