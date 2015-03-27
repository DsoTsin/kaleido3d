#include "Kaleido3D.h"
#include "Renderer.h"
#include "Device.h"
#include "Helper.h"

#include <d3d12sdklayers.h>
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
			/* Create D3D Device */
			ThrowIfFailed(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(mDevice.GetInitReference())));
			D3D12_FEATURE_DATA_D3D12_OPTIONS options;
			ThrowIfFailed(
				mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options),sizeof(options))
				);
			int nodeCount = mDevice->GetNodeCount();
			Debug::Out("Device", "node count is %d.", nodeCount);
			/* Create Command Allocator */
			ThrowIfFailed(
				mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetInitReference()))
				);

			D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
			commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			/* Create Command Queue */
			ThrowIfFailed(
				mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(mCommandQueue.GetInitReference()))
				);
			/* Create Swap Chain */
			DXGI_SWAP_CHAIN_DESC swapChainDesc = SwapChainDesc(reinterpret_cast<HWND>(window->GetHandle()));
			IDXGIFactory2 *dxgiFactory = nullptr;
			ThrowIfFailed(::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));
			ThrowIfFailed(dxgiFactory->CreateSwapChain(mCommandQueue, &swapChainDesc, (IDXGISwapChain**)mSwapChain.GetInitReference()));
			dxgiFactory->Release();
			if (swapChainDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
			{
				mSwapChain->SetMaximumFrameLatency(sBufferCount);
			}
			ThrowIfFailed(mRTVDescriptorHeap.Create(mDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, sBufferCount));
			ThrowIfFailed(mDevice->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, nullptr, IID_PPV_ARGS(mGfxCmdList.GetInitReference())));
			ThrowIfFailed(CreateRenderTargets(window, mGfxCmdList));
			
			mHandle = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
			ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetInitReference())));

			WaitForCommandQueueFence();
			mCommandAllocator->Reset();
			mGfxCmdList->Reset(mCommandAllocator, nullptr);
			Debug::Out("Device", "Init: Succeed to Init Device.");
		}

		DXGI_SWAP_CHAIN_DESC Device::SwapChainDesc(HWND hWnd) {
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
			swapChainDesc.BufferCount = sBufferCount;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = hWnd;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.Flags = 0;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			return swapChainDesc;
		}

		HRESULT Device::CreatePSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(psoDesc));
			psoDesc.NumRenderTargets = 1;
			HRESULT hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPSO.GetInitReference()));
			return hr;
		}

		HRESULT Device::CreateRenderTargets(Window * window, PtrGfxCmdList list)
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
				Debug::Out("Device", "Render Width:%d Height:%d", (int)mViewPort.Width, (int)mViewPort.Height);
				mSwapChain->ResizeBuffers(sBufferCount, (UINT)mViewPort.Width, (UINT)mViewPort.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
				ZeroMemory(&rtvDesc, sizeof(rtvDesc));
				rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				for (UINT i = 0; i < sBufferCount; ++i)
				{
					hr = mSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (LPVOID*)&mRenderTarget[i]);
					mRenderTarget[0]->SetName(L"mRenderTarget" + i);
					mDevice->CreateRenderTargetView(mRenderTarget[i], &rtvDesc, mRTVDescriptorHeap.hCPU(i));
				}
				mRectScissor = { 0, 0, static_cast<LONG>(mViewPort.Width), static_cast<LONG>(mViewPort.Height) };
			}
			UINT backBufferIndex = 0;
			backBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
			// get active RTV from descriptor heap wrapper class by index
			D3D12_CPU_DESCRIPTOR_HANDLE rtv = mRTVDescriptorHeap.hCPU(backBufferIndex);
			list->RSSetViewports(1, &mViewPort);
			list->RSSetScissorRects(1, &mRectScissor);
			hr = list->Close();
			ID3D12GraphicsCommandList* cmdList = list.GetReference();
			mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
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

		void Device::Clear() {
			UINT backBufferIndex = 0;
			backBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
			D3D12_CPU_DESCRIPTOR_HANDLE rtv = mRTVDescriptorHeap.hCPU(backBufferIndex);
			mGfxCmdList->RSSetViewports(1, &mViewPort);
			mGfxCmdList->RSSetScissorRects(1, &mRectScissor);

			Helper::SetResourceBarrier(mGfxCmdList, mRenderTarget[backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			
			float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
			mGfxCmdList->ClearRenderTargetView(rtv, clearColor, NULL, 0);
			mGfxCmdList->OMSetRenderTargets(1, &rtv, TRUE, nullptr);

			Helper::SetResourceBarrier(mGfxCmdList, mRenderTarget[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			ThrowIfFailed(mGfxCmdList->Close());
			mCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&mGfxCmdList);
			DXGI_PRESENT_PARAMETERS params;
			ZeroMemory(&params, sizeof(params));
			mSwapChain->Present(0, 0);

			WaitForCommandQueueFence();
		}

		void Device::Destroy()
		{
		}
	}
}