#include "Kaleido3D.h"
#include "Renderer.h"
#include "Device.h"
#include "Helper.h"

#include <d3d12sdklayers.h>
#include <assert.h>
#include <Config/OSHeaders.h>
#include <Core/Window.h>
#include <Core/LogUtil.h>


namespace k3d {
    namespace d3d12 {

        Device::~Device() {
        }

        void Device::Init(Window *window) {
            assert(window != nullptr && window->GetHandle() != nullptr && "window is not initialized!");
            m_Window = window;

#if defined(_DEBUG)
            Ref<ID3D12Debug> dbgController;
            ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(dbgController.GetInitReference())));
            dbgController->EnableDebugLayer();
#endif

            /* Create D3D Device */
            {
                ThrowIfFailed(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_D3DDevice.GetInitReference())));
                D3D12_FEATURE_DATA_D3D12_OPTIONS options;
                ThrowIfFailed(
                    m_D3DDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void *>(&options), sizeof(options))
                );
                int nodeCount = m_D3DDevice->GetNodeCount();
                Debug::Out("Device", "node count is %d.", nodeCount);
            }

            /* Create Command Allocator and Queue*/
            {
                D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
                commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                ThrowIfFailed(m_D3DDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetInitReference())));

                for (UINT n = 0; n < sBufferCount; n++) {
                    ThrowIfFailed(m_D3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[n].GetInitReference())));
                }
            }

            /* Create Synchronization Objects */
            {
                ThrowIfFailed(m_D3DDevice->CreateFence(m_FenceValues[m_CurrentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetInitReference())));
                m_FenceValues[m_CurrentFrame]++;
                m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
            }

            /* Create Swap Chain */
            {
                DXGI_SWAP_CHAIN_DESC swapChainDesc = SwapChainDesc(reinterpret_cast<HWND>(window->GetHandle()));
                IDXGIFactory2 *dxgiFactory = nullptr;
                ThrowIfFailed(::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));
                ThrowIfFailed(dxgiFactory->CreateSwapChain(m_CommandQueue, &swapChainDesc, (IDXGISwapChain **)m_SwapChain.GetInitReference()));
                dxgiFactory->Release();
                if (swapChainDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT) {
                    m_SwapChain->SetMaximumFrameLatency(sBufferCount);
                }
                ThrowIfFailed(m_RTVDescriptorHeap.Create(m_D3DDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, sBufferCount));
                //	ThrowIfFailed(mDevice->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator, nullptr, IID_PPV_ARGS(mGfxCmdList.GetInitReference())));
                ThrowIfFailed(CreateRenderTargets(window));
            }

            WaitForGpu();

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

        HRESULT Device::CreateRenderTargets(Window *window) {
            ZeroMemory(&m_ViewPort, sizeof(D3D12_VIEWPORT));
            RECT rc;
            GetClientRect(reinterpret_cast<HWND>(window->GetHandle()), &rc);
            m_ViewPort.TopLeftX = 0;
            m_ViewPort.TopLeftY = 0;
            m_ViewPort.Width = (float)rc.right - rc.left;
            m_ViewPort.Height = (float)rc.bottom - rc.top;
            HRESULT hr;
            if (m_ViewPort.Width > 0) {
                Debug::Out("Device", "Render Width:%d Height:%d", (int)m_ViewPort.Width, (int)m_ViewPort.Height);
                m_SwapChain->ResizeBuffers(sBufferCount, (UINT)m_ViewPort.Width, (UINT)m_ViewPort.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
                ZeroMemory(&rtvDesc, sizeof(rtvDesc));
                rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                for (UINT i = 0; i < sBufferCount; ++i) {
                    hr = m_SwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (LPVOID *)&m_RenderTarget[i]);
                    m_RenderTarget[0]->SetName(L"mRenderTarget" + i);
                    m_D3DDevice->CreateRenderTargetView(m_RenderTarget[i], &rtvDesc, m_RTVDescriptorHeap.hCPU(i));
                    m_FenceValues[i] = m_FenceValues[m_CurrentFrame];
                }
                m_RectScissor = { 0, 0, static_cast<LONG>(m_ViewPort.Width), static_cast<LONG>(m_ViewPort.Height) };
            }

            return hr;
        }

        Device::Device()
            : m_D3DDevice(nullptr)
            , m_SwapChain(nullptr)
            , m_Window(nullptr)
            , m_CurrentFrame(0) {
            ZeroMemory(m_FenceValues, sizeof(m_FenceValues));
        }

        void Device::MoveToNextFrame() {
            const UINT64 currentFenceValue = m_FenceValues[m_CurrentFrame];
            ThrowIfFailed(m_CommandQueue->Signal(m_Fence, currentFenceValue));

            // Advance the frame index.
            m_CurrentFrame = (m_CurrentFrame + 1) % sBufferCount;

            // Check to see if the next frame is ready to start.
            if (m_Fence->GetCompletedValue() < m_FenceValues[m_CurrentFrame]) {
                ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_CurrentFrame], m_FenceEvent));
                WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
            }

            // Set the fence value for the next frame.
            m_FenceValues[m_CurrentFrame] = currentFenceValue + 1;
        }

        void Device::WaitForGpu() {
            ThrowIfFailed(m_CommandQueue->Signal(m_Fence, m_FenceValues[m_CurrentFrame]));

            ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_CurrentFrame], m_FenceEvent));
            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
            m_FenceValues[m_CurrentFrame]++;
        }

		void Device::Present()
		{
			HRESULT hr = m_SwapChain->Present(1, 0);
			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				//m_deviceRemoved = true;
			}
			else
			{
				ThrowIfFailed(hr);
				MoveToNextFrame();
			}
		}

        void Device::Destroy() {
        }



		PtrDevice Device::GetD3DDevice() {
			return m_D3DDevice;
		}

		PtrSwapChain Device::GetSwapChain() {
			return m_SwapChain;
		}

		PtrCmdQueue Device::GetDefaultCommandQueue() {
			return m_CommandQueue;
		}

		PtrCmdAllocator Device::GetCommandAllocator()
		{
			return m_CommandAllocators[m_CurrentFrame];
		}
    }
}