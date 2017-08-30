#pragma once
#include <Kaleido3D.h>
#include <dwrite.h>
#include <d2d1_3.h>
#include <d3d11on12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <d3d12shader.h>
#include <d3d12sdklayers.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <dxgi1_4.h>

#include "Utils/d3dx12.h"

#include <Core/Os.h>
#include <Core/Dispatch/Dispatcher.h>
#include <Core/LogUtil.h>

#include <Math/kMath.hpp>
#ifdef ENABLE_SSE
#include <Math/kMath_SSE.hpp>
#endif

#include <memory>
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>

#define DX12LOG(level, ...) KLOG(level, kaleido3d::D3D12RHI, __VA_ARGS__)

#define RDX_CHECK_ERR(expression, message, errproc) \
  if (FAILED ((expression))) \
  {\
    KLOG(Error, "DX12_ERR", #expression ##" " ## message); \
    errproc; \
  }

#define RDX_ERR(expression, errproc) \
  if (FAILED ((expression))) \
  {\
    KLOG(Error, "DX12_ERR", "failed "## #expression ##" %s %d", __FILE__, __LINE__); \
    errproc; \
  }

#define ThrowIfFailed(expr) \
	do { \
		if (FAILED(expr)) { \
		KLOG(Fatal, "DX12_ERR", "failed "## #expr ##" %s %d", __FILE__, __LINE__); \
		throw; \
		}\
	} while (0);

#define NS_K3D_D3D12_BEGIN namespace k3d { \
namespace d3d12 {

#define NS_K3D_D3D12_END }}

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL 0ull
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ~0ull

namespace k3d
{
	using namespace Microsoft::WRL;
	using namespace DirectX;
	using Vector3f = DirectX::XMFLOAT3;
	using Vector4f = DirectX::XMVECTOR;
	using Matrix4f = DirectX::XMMATRIX;
	using std::shared_ptr;
	using std::vector;

	namespace d3d12	{
		/** \brief pointer alias. */
		using PtrDevice = ComPtr < ID3D12Device >;
		using PtrDevice11 = ComPtr < ID3D11Device >;
		using PtrDevice11On12 = ComPtr <ID3D11On12Device >;
		using PtrDevice2D = ComPtr< ID2D1Device2 >;
		using PtrDevice2DContext = ComPtr<ID2D1DeviceContext2>;
		using PtrD2DFactory = ComPtr<ID2D1Factory3>;
		using PtrDXGIDevice = ComPtr < IDXGIDevice3 >;
		using PtrAdapter = ComPtr < IDXGIAdapter1 >;
		using PtrSwapChain = ComPtr < IDXGISwapChain3 >;
		using PtrGIFactory = ComPtr<IDXGIFactory4>;
		using PtrGfxCmdList = ComPtr < ID3D12GraphicsCommandList >;
		using PtrCmdList = ComPtr < ID3D12CommandList >;
		using PtrCmdQueue = ComPtr < ID3D12CommandQueue >;
		using PtrCmdAllocator = ComPtr < ID3D12CommandAllocator >;
		using PtrPipeLineState = ComPtr < ID3D12PipelineState >;
		using PtrResource = ComPtr < ID3D12Resource >;
		using PtrDescHeap = ComPtr < ID3D12DescriptorHeap >;
		using PtrBlob = ComPtr < ID3DBlob >;
		using PtrFence = ComPtr< ID3D12Fence >;
		using PtrRootSignature = ComPtr<ID3D12RootSignature>;
		using PtrSolidColorBrush = ComPtr<ID2D1SolidColorBrush>;
		using PtrTextFormat = ComPtr<IDWriteTextFormat>;
		using PtrCommandSignature = ComPtr<ID3D12CommandSignature>;
		using Vector4F = kMath::tVectorN<float, 4>;
		
		enum class MeshSemantic {
			VertexData,
			NormalData,
			TangentData,
			TexCoordData,
			IndiceData
		};

		/**
		* \brief	divide jobs into asynchronous threads.
		*/
		enum class AsyncThread {
			Resource,
			PSO,
			CommandList,
			MasterRender
		};

		using Os::Thread;
		using Dispatch::WorkQueue;

	}
}