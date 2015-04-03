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
		Device::~Device ()
		{
		}

		void Device::Init (Window *window)
		{
			assert (window!=nullptr && window->GetHandle ()!=nullptr && "window is not initialized!");
			ZeroMemory (&mViewPort, sizeof (D3D12_VIEWPORT));
			RECT rc;
			GetClientRect (reinterpret_cast<HWND>(window->GetHandle ()), &rc);
			mViewPort.TopLeftX = 0;
			mViewPort.TopLeftY = 0;
			mViewPort.Width = (float)rc.right-rc.left;
			mViewPort.Height = (float)rc.bottom-rc.top;
			RDX_CHECK_ERR (CreateDevice (), "Failed to create device.", return);
			RDX_CHECK_ERR (CreateCmdAllocator (), "Failed to create CMDAllocator.", return);
			RDX_CHECK_ERR (CreateSwapChain (window), "Failed to create SwapChain.", return);
			RDX_CHECK_ERR (CreateDescHeap (), "failed to create desciptor heap.", return);
			//K3D_CHECK_DX_ERROR(CreatePSO(), "Failed to create PSO.", return);
			RDX_CHECK_ERR (CreateGfxCmdList (), "Failed to CreateGfxCmdList.", return);
			RDX_CHECK_ERR (CreateRTV (), "Failed to create RTV.", return);
			RDX_CHECK_ERR (CreateFence (), "Failed to create fence.", return);
			WaitForCommandQueueFence ();
			mCommandAllocator->Reset ();
			mGfxCmdList->Reset (mCommandAllocator, nullptr);
			Debug::Out ("Device", "Init: Succeed to Init Device.");
		}

		HRESULT Device::CreateDevice ()
		{
			HRESULT	hr =
				D3D12CreateDevice (
				NULL,                                           /** Use the Default DXGI Adapter **/
				D3D_DRIVER_TYPE_HARDWARE,
				D3D12_CREATE_DEVICE_DEBUG,
				D3D_FEATURE_LEVEL_11_0,
				D3D12_SDK_VERSION,
        IID_PPV_ARGS (mDevice.GetInitReference ()));

			if (FAILED (hr))
			{
				Log::Error ("Device::Init D3D12CreateDevice failed. in file(%s), line(%d).", __FILE__, __LINE__);
				Debug::Out ("Device", "Init: D3D12CreateDevice failed.");
			}
			else
			{
				Debug::Out ("Device", "Init: D3D12CreateDevice successful!!");
        Helper::CheckHWFeatures(mDevice);
			}
			return hr;
		}

		HRESULT Device::CreateCmdAllocator ()
		{
			assert (mDevice!=nullptr);
			HRESULT hr = mDevice->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (mCommandAllocator.GetInitReference ()));
			if (FAILED (hr))
			{
				Debug::Out ("Device", "failed to CreateCommandAllocator");
				return hr;
			}
			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			hr = mDevice->CreateCommandQueue (&commandQueueDesc, IID_PPV_ARGS (mCommandQueue.GetInitReference ()));
			return hr;
		}

		HRESULT Device::CreateSwapChain (Window * window)
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory (&swapChainDesc, sizeof (swapChainDesc));
			swapChainDesc.BufferCount = 2;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = reinterpret_cast<HWND>(window->GetHandle ());
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			IDXGIFactory2 *dxgiFactory = nullptr;
			HRESULT hr = ::CreateDXGIFactory2 (0, IID_PPV_ARGS (&dxgiFactory));
			if (FAILED (hr))
			{
				Debug::Out ("Device", "failed to create dxgi factory..");
				return hr;
			}
			hr = dxgiFactory->CreateSwapChain (mCommandQueue, &swapChainDesc, mSwapChain.GetInitReference ());
			dxgiFactory->Release ();
			if (FAILED (hr))
			{
				Debug::Out ("Device", "Failed to create CreateSwapChain..");
			}
			return hr;
		}

		HRESULT Device::CreateDescHeap ()
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
			ZeroMemory (&heapDesc, sizeof (D3D12_DESCRIPTOR_HEAP_DESC));
			heapDesc.NumDescriptors = 2;
			heapDesc.Type = D3D12_RTV_DESCRIPTOR_HEAP;
			HRESULT hr = mDevice->CreateDescriptorHeap (&heapDesc, IID_PPV_ARGS (mDescriptorHeap.GetInitReference ()));
			mRenderTargetView[0] = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart ();
			UINT HandleIncrementSize = mDevice->GetDescriptorHandleIncrementSize (D3D12_RTV_DESCRIPTOR_HEAP);
			mRenderTargetView[1] = mRenderTargetView[0].MakeOffsetted (1, HandleIncrementSize);
			return hr;
		}

		HRESULT Device::CreatePSO ()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory (&psoDesc, sizeof (psoDesc));
			psoDesc.NumRenderTargets = 1;
			// Create the actual PSO.
			HRESULT hr = mDevice->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (mPSO.GetInitReference ()));
			if (FAILED (hr))
			{
				Debug::Out ("Device", "Failed to CreateGraphicsPipelineState.");
			}
			return hr;
		}

		HRESULT Device::CreateGfxCmdList ()
		{
			HRESULT hr = mDevice->CreateCommandList (1, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, nullptr, IID_PPV_ARGS (mGfxCmdList.GetInitReference ()));
			return hr;
		}

		HRESULT Device::CreateRTV ()
		{
			HRESULT hr = mSwapChain->GetBuffer (0, IID_PPV_ARGS (mRenderTarget[0].GetInitReference ()));
			mRenderTarget[0]->SetName (L"mRenderTarget0");  //set debug name 
			mDevice->CreateRenderTargetView (mRenderTarget[0], nullptr, mRenderTargetView[0]);
			hr = mSwapChain->GetBuffer (1, IID_PPV_ARGS (mRenderTarget[1].GetInitReference ()));
			mRenderTarget[1]->SetName (L"mRenderTarget1");
			mDevice->CreateRenderTargetView (mRenderTarget[1], nullptr, mRenderTargetView[1]);
			mGfxCmdList->RSSetViewports (1, &mViewPort);
			hr = mGfxCmdList->Close ();
			ID3D12GraphicsCommandList* cmdList = mGfxCmdList.GetReference ();
			mCommandQueue->ExecuteCommandLists (1, (ID3D12CommandList* const*)&cmdList);
			return hr;
		}

		HRESULT Device::CreateFence ()
		{
			mHandle = ::CreateEvent (nullptr, FALSE, FALSE, nullptr);
			HRESULT hr = mDevice->CreateFence (0, D3D12_FENCE_MISC_NONE, IID_PPV_ARGS (mFence.GetInitReference ()));
			return hr;
		}

		Device::Device ()
			: mDevice (nullptr)
			, mSwapChain (nullptr)
		{
		}

		void Device::WaitForCommandQueueFence ()
		{
			mFence->Signal (0);
			mFence->SetEventOnCompletion (1, mHandle);
			mCommandQueue->Signal (mFence, 1);
			WaitForSingleObject (mHandle, INFINITE);
		}

		PtrDevice Device::GetDevice ()
		{
			return mDevice;
		}

		void Device::Destroy ()
		{
		}
	}
}