#pragma once
#ifndef __D3D12Viewport_h__
#define __D3D12Viewport_h__
#include "D3D12RHI.h"

NS_K3D_D3D12_BEGIN

class D3D12Viewport : public D3D12RHIDeviceChild, public rhi::IRenderViewport
{
public:
	D3D12Viewport(rhi::IDevice* pDevice, void* WindowHandle) : D3D12RHIDeviceChild(static_cast<Device::Ptr>(pDevice)) {}

	D3D12Viewport(Device::Ptr pDevice, HWND WindowHandle, rhi::GfxSetting&);

	~D3D12Viewport()
	{
	}

	bool							InitViewport(
										void *windowHandle, 
										rhi::IDevice * pDevice, 
										rhi::GfxSetting&);

	void							Resize(uint32 Width, uint32 Height);
	bool							Present(bool VSync = true) override;

	PtrResource						GetBackBuffer() const { return m_BackBuffers[GetBackBufferIndex()]; }
	D3D12_CPU_DESCRIPTOR_HANDLE		GetBackBufferHandle() const { return m_BackBuffersHandle[GetBackBufferIndex()]; }
	SIZE_T							GetBackBufferIndex() const { return m_SwapChain->GetCurrentBackBufferIndex(); }

	rhi::IRenderTarget*				GetRenderTarget(uint32 index) override;
	PtrSwapChain					GetSwapChain() const { return m_SwapChain; }


	uint32							GetSwapChainIndex() override { return m_SwapChain->GetCurrentBackBufferIndex(); }
	uint32							GetSwapChainCount() override { return DefaultNumBackBuffers; }

	uint32							GetWidth() const override { return 0; }
	uint32							GetHeight() const override { return 0; }

private:
	K3D_DISCOPY(D3D12Viewport);

	static const int				DefaultNumBackBuffers = 3;
	DXGI_SWAP_CHAIN_DESC			m_SwapChainDesc;
	HWND							m_WindowHandle;
	PtrSwapChain					m_SwapChain;
	PtrResource						m_BackBuffers[DefaultNumBackBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_BackBuffersHandle[DefaultNumBackBuffers];
	uint64							m_LastFrameComplete;
};


NS_K3D_D3D12_END

#endif