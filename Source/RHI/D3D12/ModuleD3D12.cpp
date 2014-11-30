#include "DXCommon.h"
#include "ModuleD3D12.h"
#include "RHI.h"
#include "Public/D3D12RHI.h"
#include <Core>

NS_K3D_D3D12_BEGIN

#define SWAP_CHAIN_BUFFER_COUNT 3

using namespace rhi;

namespace ModuleD3D12
{
	DXGI_FORMAT			SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorBuffer			g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];

	shared_ptr<Device>	primaryDevice;
	PtrSwapChain		primarySwapChain;

	bool InitDevice(void)
	{
		if (!primaryDevice.get()) 
		{
			IDeviceAdapter ** list = nullptr;
			uint32 adapterNum = 0;
			EnumAllDeviceAdapter(list, &adapterNum);
			primaryDevice = std::make_shared<Device>();
#ifdef _DEBUG
			primaryDevice->Create(list[0], true);
#else
			primaryDevice->Create(list[0], false);
#endif
		}
			
		RHIRoot::Init(primaryDevice);
		return true;
	}

	bool InitSurface(Window *pWindow) {
		if (primarySwapChain.Get()) 
		{
			primarySwapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, pWindow->Width(), pWindow->Height(), SwapChainFormat, 0);
			return true;
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = pWindow->Width();
			swapChainDesc.BufferDesc.Height = pWindow->Height();
			swapChainDesc.BufferDesc.Format = SwapChainFormat;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;// | DXGI_USAGE_UNORDERED_ACCESS;
			swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			swapChainDesc.OutputWindow = (HWND)pWindow->GetHandle();
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			ThrowIfFailed(primaryDevice->GetDXGIFactory()->CreateSwapChain(RHIRoot::GetPrimaryCommandListManager().GetCommandQueue(), &swapChainDesc, (IDXGISwapChain**)primarySwapChain.GetAddressOf()));
			
			for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
			{
				ComPtr<ID3D12Resource> DisplayPlane;
				ThrowIfFailed(primarySwapChain->GetBuffer(i, IID_PPV_ARGS(&DisplayPlane)));
				g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
			}

			GraphicsContext & InitContext = GraphicsContext::Begin(
				RHIRoot::GetPrimaryCommandListManager());

			InitContext.CloseAndExecute();
		}
		return true;
	}
}

NS_K3D_D3D12_END