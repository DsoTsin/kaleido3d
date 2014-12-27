#include <assert.h>
#include <d3d11_2.h>

#include <Config/OSHeaders.h>
#include <Core/Window.h>
#include <Core/LogUtil.h>

#include "DirectXRenderer.h"


using namespace k3d;

DirectXContext::~DirectXContext() {}

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

DirectXContext * DirectXContext::CreateContext(Window * window, DXFeature feature) {
	assert(window!=nullptr && window->GetHandle()!=nullptr && "window is not initialized!");

	HWND hWnd = reinterpret_cast<HWND>(window->GetHandle());

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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

	DirectXContext * context = new DirectXContext(window);
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
		featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
		&sd, &(context->pSwapChain), &(context->pDevice), &preference.featureLevel, &(context->pImmediateContext));

	if (FAILED(hr)) {
		return nullptr;
	}
	kDebug("D3D11CreateDeviceAndSwapChain successful!!\n");

	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	hr = context->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return nullptr;

	hr = context->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &(context->pRenderTargetView));
	pBackBuffer->Release();
	if (FAILED(hr))
		return nullptr;
	kDebug("CreateRenderTargetView successful!!\n");

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
	hr = context->pDevice->CreateTexture2D(&descDepth, NULL, &(context->pDepthStencil));
	if (FAILED(hr))
		return nullptr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = context->pDevice->CreateDepthStencilView(context->pDepthStencil, &descDSV, &(context->pDepthStencilView));
	if (FAILED(hr))
		return nullptr;
	kDebug("CreateDepthStencilView successful!!\n");

	context->pImmediateContext->OMSetRenderTargets(1, &(context->pRenderTargetView), context->pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->pImmediateContext->RSSetViewports(1, &vp);

	return context;
}

DirectXContext::DirectXContext(Window *)
: pDevice(nullptr)
, pSwapChain(nullptr)
, pRenderTargetView(nullptr)
, pImmediateContext(nullptr)
, pDepthStencil(nullptr)
, pDepthStencilView(nullptr)
{}

void DirectXContext::Destroy() {
	if (pImmediateContext)	pImmediateContext->ClearState();
	if (pDepthStencil)		pDepthStencil->Release();
	if (pDepthStencilView)	pDepthStencilView->Release();
	if (pRenderTargetView)	pRenderTargetView->Release();
	if (pSwapChain)			pSwapChain->Release();
	if (pImmediateContext)	pImmediateContext->Release();
	if (pDevice)			pDevice->Release();
}