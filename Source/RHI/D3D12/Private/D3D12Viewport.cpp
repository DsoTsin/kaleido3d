#include "DXCommon.h"
#include "Public/D3D12Viewport.h"
#include <Core/LogUtil.h>

NS_K3D_D3D12_BEGIN

D3D12Viewport::D3D12Viewport(Device::Ptr pDevice, HWND WindowHandle, uint32 Width, uint32 Height)
	: D3D12RHIDeviceChild(pDevice)
	, m_WindowHandle(WindowHandle)
{
	m_SwapChainDesc.OutputWindow = WindowHandle;

	m_SwapChainDesc.BufferDesc.Width = Width;
	m_SwapChainDesc.BufferDesc.Height = Height;
	m_SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.BufferCount = DefaultNumBackBuffers;

	m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapChainDesc.Windowed = TRUE;
	Log::Out(LogLevel::Info, "D3D12Viewport", "D3D12_Initialized: width(%d), height(%d).", Width, Height);
}

bool D3D12Viewport::InitViewport(
	void *windowHandle, 
	rhi::IDevice * pDevice, 
	uint32 width, uint32 height, 
	rhi::EPixelFormat rtFmt)
{
	PtrGIFactory Factory = GetParentDeviceRef().GetDXGIFactory();
	ID3D12CommandQueue * pCmdQueue = GetParentDeviceRef().GetDefaultContext()
		.GetCommandListManager().GetCommandQueue();
	ComPtr<IDXGISwapChain> swapChain;
	ThrowIfFailed(Factory->CreateSwapChain(
		pCmdQueue,
		&m_SwapChainDesc,
		swapChain.GetAddressOf()
		));
	ThrowIfFailed(swapChain.As(&m_SwapChain));
	DescriptorHeapAllocator & dha = GetParentDeviceRef().GetViewDescriptorAllocator<D3D12_RENDER_TARGET_VIEW_DESC>();
	SIZE_T outIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = dha.AllocateHeapSlot(outIndex);
	for (int n = 0; n < DefaultNumBackBuffers; n++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(m_BackBuffers[n].GetAddressOf())));
		GetParentDeviceRef().Get()->CreateRenderTargetView(m_BackBuffers[n].Get(), nullptr, handle);
		m_BackBuffersHandle[n] = dha.AllocateHeapSlot(outIndex);
	}
	return true;
}

void D3D12Viewport::Resize(uint32 Width, uint32 Height)
{
}

bool D3D12Viewport::Present(bool VSync)
{
	m_SwapChain->Present(1, 0);
	return false;
}

NS_K3D_D3D12_END