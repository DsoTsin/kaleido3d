#include "DXCommon.h"
#include "Public/D3D12Viewport.h"
#include "D3D12Enums.h"
#include <Core/LogUtil.h>

NS_K3D_D3D12_BEGIN

D3D12Viewport::D3D12Viewport(Device::Ptr pDevice, HWND WindowHandle, rhi::GfxSetting &setting)
	: D3D12RHIDeviceChild(pDevice)
	, m_WindowHandle(WindowHandle)
{
	m_SwapChainDesc.OutputWindow = WindowHandle;

	m_SwapChainDesc.BufferDesc.Width = setting.Width;
	m_SwapChainDesc.BufferDesc.Height = setting.Height;
	m_SwapChainDesc.BufferDesc.Format = g_DXGIFormatTable[setting.ColorFormat];

	m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.BufferCount = setting.BackBufferCount;

	m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapChainDesc.Windowed = TRUE;
	DX12LOG(Info,"D3D12Viewport_Initialized: width(%d), height(%d).", setting.Width, setting.Height);
}

bool D3D12Viewport::InitViewport(
	void *windowHandle, 
	rhi::IDevice * pDevice, 
	rhi::GfxSetting& setting)
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

rhi::RenderTargetRef D3D12Viewport::GetRenderTarget(uint32 index)
{
	return rhi::RenderTargetRef();
}

NS_K3D_D3D12_END