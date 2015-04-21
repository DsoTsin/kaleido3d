#include "Kaleido3D.h"
#include "Renderer.h"
#include "Device.h"
#include "Helper.h"

#include <assert.h>
#include <Config/OSHeaders.h>
#include <Core/Window.h>
#include <Core/LogUtil.h>


namespace k3d
{
	namespace d3d12
	{

		Device::~Device()
		{
		}

		void Device::Init(Window *window)
		{
			assert(window != nullptr && window->GetHandle() != nullptr && "window is not initialized!");
			mWindow = window;
			RDX_CHECK_ERR(CreateDevice(), "Failed to create device.", return);

			int nodeCount = mDevice->GetNodeCount();
			Debug::Out("Device", "node count is %d.", nodeCount);

			RDX_CHECK_ERR(CreateCmdAllocatorAndQueue(), "Failed to create CMDAllocator.", return);
			RDX_CHECK_ERR(CreateSwapChain(window), "Failed to create SwapChain.", return);
			RDX_CHECK_ERR(CreateDescHeap(), "failed to create desciptor heap.", return);
			//K3D_CHECK_DX_ERROR(CreatePSO(), "Failed to create PSO.", return);
			RDX_CHECK_ERR(CreateGfxCmdList(), "Failed to CreateGfxCmdList.", return);
			RDX_CHECK_ERR(CreateRenderTargets(window), "Failed to create RTV.", return);
			RDX_CHECK_ERR(CreateFence(), "Failed to create fence.", return);
			WaitForCommandQueueFence();
			mCommandAllocator->Reset();
			mGfxCmdList->Reset(mCommandAllocator, nullptr);
			Debug::Out("Device", "Init: Succeed to Init Device.");
		}


		HRESULT Device::CreateDevice()
		{
			HRESULT	hr =
				D3D12CreateDevice(
					NULL,
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(mDevice.GetInitReference()));
			return hr;
		}

		HRESULT Device::CreateCmdAllocatorAndQueue()
		{
			assert(mDevice != nullptr);
			HRESULT hr = mDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(mCommandAllocator.GetInitReference()));
			if (FAILED(hr))
			{
				Debug::Out("Device", "failed to CreateCommandAllocator");
				return hr;
			}
			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

			hr = mDevice->CreateCommandQueue(
				&commandQueueDesc,
				IID_PPV_ARGS(mCommandQueue.GetInitReference()));

			return hr;
		}

		HRESULT Device::CreateSwapChain(Window * window)
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
			swapChainDesc.BufferCount = sBufferCount;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = reinterpret_cast<HWND>(window->GetHandle());
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
			IDXGIFactory2 *dxgiFactory = nullptr;
			HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
			if (FAILED(hr))
			{
				Debug::Out("Device", "failed to create dxgi factory..");
				return hr;
			}
			hr = dxgiFactory->CreateSwapChain(mCommandQueue, &swapChainDesc, (IDXGISwapChain**)mSwapChain.GetInitReference());
			dxgiFactory->Release();
			if (FAILED(hr))
			{
				Debug::Out("Device", "Failed to create CreateSwapChain..");
			}
			if (swapChainDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
			{
				mSwapChain->SetMaximumFrameLatency(sBufferCount);
			}
			return hr;
		}

		HRESULT Device::CreateDescHeap()
		{
			HRESULT hr = mRTVDescriptorHeap.Create(mDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, sBufferCount);
			return hr;
		}

		HRESULT Device::CreatePSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(psoDesc));
			psoDesc.NumRenderTargets = 1;
			HRESULT hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPSO.GetInitReference()));
			return hr;
		}

		HRESULT Device::CreateGfxCmdList()
		{
			HRESULT hr = mDevice->CreateCommandList(
				1, D3D12_COMMAND_LIST_TYPE_DIRECT,
				mCommandAllocator,
				nullptr,
				IID_PPV_ARGS(mGfxCmdList.GetInitReference()));
			return hr;
		}

		HRESULT Device::CreateRenderTargets(Window * window)
		{
			ZeroMemory(&mViewPort, sizeof(D3D12_VIEWPORT));
			RECT rc;
			GetClientRect(reinterpret_cast<HWND>(window->GetHandle()), &rc);
			mViewPort.TopLeftX = 0;
			mViewPort.TopLeftY = 0;
			mViewPort.Width = (float)rc.right - rc.left;
			mViewPort.Height = (float)rc.bottom - rc.top;

			HRESULT hr;
			//if the client size is valid (ignore minimize etc).
			if (mViewPort.Width > 0)
			{
				//debug output of new buffer size
				std::string resizeString = "Backbuffer size: ";
				resizeString.append(std::to_string(mViewPort.Width));
				resizeString.append("x");
				resizeString.append(std::to_string(mViewPort.Height));
				resizeString.append("\n");
				OutputDebugString(resizeString.c_str());
				mSwapChain->ResizeBuffers(sBufferCount, mViewPort.Width, mViewPort.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
				ZeroMemory(&rtvDesc, sizeof(rtvDesc));
				rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

				//loop for all backbuffer resources
				for (UINT i = 0; i < sBufferCount; ++i)
				{
					hr = mSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (LPVOID*)&mRenderTarget[i]);
					mRenderTarget[0]->SetName(L"mRenderTarget" + i);
					mDevice->CreateRenderTargetView(mRenderTarget[i], &rtvDesc, mRTVDescriptorHeap.hCPU(i));
				}

				//mRectScissor = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
			}
			UINT backBufferIndex = 0;
			backBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
			//get active RTV from descriptor heap wrapper class by index
			D3D12_CPU_DESCRIPTOR_HANDLE rtv = mRTVDescriptorHeap.hCPU(backBufferIndex);

			mGfxCmdList->RSSetViewports(1, &mViewPort);
			//mGfxCmdList->RSSetScissorRects(1, &mRectScissor);
			hr = mGfxCmdList->Close();
			ID3D12GraphicsCommandList* cmdList = mGfxCmdList.GetReference();
			mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
			return hr;
		}

		HRESULT Device::CreateFence()
		{
			mHandle = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
			HRESULT hr = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetInitReference()));
			return hr;
		}

		Device::Device()
			: mDevice(nullptr)
			, mSwapChain(nullptr)
			, mWindow(nullptr)
		{
		}

		void Device::WaitForCommandQueueFence()
		{
			mFence->Signal(0);
			mFence->SetEventOnCompletion(1, mHandle);
			mCommandQueue->Signal(mFence, 1);
			WaitForSingleObject(mHandle, INFINITE);
		}

		PtrDevice Device::GetD3DDevice()
		{
			return mDevice;
		}

		void Device::Destroy()
		{
		}
	}
}