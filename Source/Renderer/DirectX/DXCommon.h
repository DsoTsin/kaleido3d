#pragma once
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <d3d12shader.h>
#include <d3d12sdklayers.h>
#include <windowsx.h>
#include <dxgi1_4.h>
#include <KTL/RefCount.hpp>


#define RDX_CHECK_ERR(expression, message, errproc) \
  if (FAILED ((expression))) \
  {\
    Debug::Out ("RDX_CHECK_ERR", #expression ##" " ## message); \
    errproc; \
  }

#define RDX_ERR(expression, errproc) \
  if (FAILED ((expression))) \
  {\
    Debug::Out ("RDX_ERR", "failed "## #expression ##" %s %d", __FILE__, __LINE__); \
    errproc; \
  }

#define ThrowIfFailed(expr) \
	do { \
		if (FAILED(expr)) { \
		Debug::Out ("RDX_ERR", "failed "## #expr ##" %s %d", __FILE__, __LINE__); \
		throw; \
		}\
	} while (0);

namespace k3d
{
	using Vector3f = DirectX::XMFLOAT3;
	using Vector4f = DirectX::XMVECTOR;
	using Matrix4f = DirectX::XMMATRIX;

	namespace d3d12
	{
		using PtrDevice = Ref < ID3D12Device >;
		using PtrDevice11 = Ref < ID3D11Device >;
		using PtrDXGIDevice = Ref < IDXGIDevice3 >;
		using PtrSwapChain = Ref < IDXGISwapChain3 >;
		using PtrGfxCmdList = Ref < ID3D12GraphicsCommandList >;
		using PtrCmdList = Ref < ID3D12CommandList >;
		using PtrCmdQueue = Ref < ID3D12CommandQueue >;
		using PtrCmdAllocator = Ref < ID3D12CommandAllocator >;
		using PtrPipeLineState = Ref < ID3D12PipelineState >;
		using PtrResource = Ref < ID3D12Resource >;
		using PtrDescHeap = Ref < ID3D12DescriptorHeap >;
		using PtrBlob = Ref < ID3DBlob >;
		using PtrFence = Ref< ID3D12Fence >;
		using PtrRootSignature = Ref<ID3D12RootSignature>;
	}
}