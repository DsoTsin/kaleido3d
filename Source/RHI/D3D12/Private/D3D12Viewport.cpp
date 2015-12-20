#include "DXCommon.h"
#include "Public/D3D12Viewport.h"
#include <Core/LogUtil.h>

NS_K3D_D3D12_BEGIN

void D3D12Viewport::Init()
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