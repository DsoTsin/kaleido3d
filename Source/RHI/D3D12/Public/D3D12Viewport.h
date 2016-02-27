#pragma once
#ifndef __D3D12Viewport_h__
#define __D3D12Viewport_h__
#include "D3D12RHI.h"

NS_K3D_D3D12_BEGIN

class D3D12Viewport : public D3D12RHIDeviceChild, public rhi::IRenderViewport
{
public:
	D3D12Viewport(rhi::IDevice* pDevice, void* WindowHandle) : D3D12RHIDeviceChild(static_cast<Device::Ptr>(pDevice)) {}

	D3D12Viewport(Device::Ptr pDevice, HWND WindowHandle,	uint32 Width, uint32 Height)
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
	}

	~D3D12Viewport()
	{
	}

	bool							InitViewport(
										void *windowHandle, 
										rhi::IDevice * pDevice, 
										uint32 width, uint32 height, 
										rhi::EPixelFormat rtFmt);

	void							Resize(uint32 Width, uint32 Height);
	bool							Present(bool VSync = true) override;

	PtrResource						GetBackBuffer() const { return m_BackBuffers[GetBackBufferIndex()]; }
	D3D12_CPU_DESCRIPTOR_HANDLE		GetBackBufferHandle() const { return m_BackBuffersHandle[GetBackBufferIndex()]; }
	SIZE_T							GetBackBufferIndex() const { return m_SwapChain->GetCurrentBackBufferIndex(); }

	PtrSwapChain					GetSwapChain() const { return m_SwapChain; }


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