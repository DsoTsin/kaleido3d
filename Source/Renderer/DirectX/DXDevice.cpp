#include "Kaleido3D.h"
#include "DirectXRenderer.h"
#include <assert.h>
#include <Config/OSHeaders.h>
#include <Core/Window.h>
#include <Core/LogUtil.h>

#include "DirectXRenderer.h"


using namespace k3d;

DXDevice::~DXDevice() {}

struct DriverPreference {
	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;
};

DriverPreference GetDriverPreference(DXFeature feature) {
	/*
		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);
	*/

	DriverPreference preference{ D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_11_1 };
	switch (feature) {
	case DXFeature::Level_11_1:
			return{ D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_11_1 };
	case DXFeature::Level_11_2:
	case DXFeature::Level_12_0:
		break;
	}
	return preference;
}

void DXDevice::Init(Window *window, DXFeature feature)
{
	assert(window != nullptr && window->GetHandle() != nullptr && "window is not initialized!");
	HWND hWnd = reinterpret_cast<HWND>(window->GetHandle());
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	UINT createDeviceFlags = 0;
#if (_WIN32_WINNT <= _WIN32_WINNT_WIN8)
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
#endif
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	DriverPreference preference = GetDriverPreference(feature);

	static D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT	hr = D3D11CreateDeviceAndSwapChain(
		NULL, preference.driverType, NULL, createDeviceFlags,
		featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd,
		pSwapChain.GetInitReference(),
		pDevice.GetInitReference(), &preference.featureLevel,
		pImmediateContext.GetInitReference());

	if (FAILED(hr)) {
		Log::Error("DXDevice::Init D3D11CreateDeviceAndSwapChain failed. in file(%s), line(%d).", __FILE__, __LINE__);
		Debug::Out("DXDevice","Init: D3D11CreateDeviceAndSwapChain failed.");
		return;
	}

	Debug::Out("DXDevice","Init: D3D11CreateDeviceAndSwapChain successful!!");

	// check for multithread support


	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)pBackBuffer.GetInitReference());
	if (FAILED(hr))
	{
		Log::Error("DXDevice::Init pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)pBackBuffer.GetInitReference()) failed. in file(%s), line(%d).", __FILE__, __LINE__);
		return;
	}

	hr = pDevice->CreateRenderTargetView(pBackBuffer, nullptr, pRenderTargetView.GetInitReference());

	if (FAILED(hr))
	{
		Log::Error("DXDevice::Init pDevice->CreateRenderTargetView failed. in file(%s), line(%d).", __FILE__, __LINE__);
		return;
	}

	Debug::Out("DXDevice","Init CreateRenderTargetView successful!!");

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	hr = pDevice->CreateTexture2D(&descDepth, nullptr, pDepthStencil.GetInitReference());
	if (FAILED(hr))
	{
		Log::Error("DXDevice::Init pDevice->CreateTexture2D failed. in file(%s), line(%d).", __FILE__, __LINE__);
		return;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = pDevice->CreateDepthStencilView(pDepthStencil, &descDSV, pDepthStencilView.GetInitReference());
	if (FAILED(hr))
	{
		Log::Error("DXDevice::Init pDevice->CreateDepthStencilView failed. in file(%s), line(%d).", __FILE__, __LINE__);
		return;
	}
	Debug::Out("DXDevice","Init: CreateDepthStencilView successful!!");
	ID3D11RenderTargetView* rtView = pRenderTargetView.GetReference();
	pImmediateContext->OMSetRenderTargets(1, &rtView, pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pImmediateContext->RSSetViewports(1, &vp);

	Debug::Out("DXDevice","Init: Succeed to Init DXDevice.");
}

DXDevice::DXDevice()
: pDevice(nullptr)
, pSwapChain(nullptr)
, pRenderTargetView(nullptr)
, pImmediateContext(nullptr)
, pDepthStencil(nullptr)
, pDepthStencilView(nullptr)
{}

void DXDevice::Destroy() {
}