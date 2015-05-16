#pragma once
#include <KTL/Singleton.hpp>
#include "DXCommon.h"
#include "Helper.h"

namespace k3d {
// Forward Declarations
    class Window;

    namespace d3d12 {
        /**
        * RenderDevice
        * manage d3d12 device-related objects
        */
        class Device : public Singleton < Device > {
          public:
            ~Device();

            void				Init(Window *);
            void				Destroy();
			void				Present();

			PtrDevice			GetD3DDevice();
			PtrSwapChain		GetSwapChain();
			PtrCmdQueue			GetDefaultCommandQueue();
			PtrCmdAllocator		GetCommandAllocator();
			PtrGfxCmdList		GetDefaultGraphicsCommandList();

          protected:
            Device();
            friend class Singleton<Device>;

          private:
			static DXGI_SWAP_CHAIN_DESC SwapChainDesc(HWND hWnd);
            HRESULT						CreateRenderTargets(Window *window);
			void						WaitForGpu();
			void						MoveToNextFrame();

          private:
            static const uint32	sBufferCount = 4;
            PtrDevice             m_D3DDevice;
            PtrSwapChain          m_SwapChain;
            PtrDXGIDevice         m_DXGIDevice;
            PtrCmdAllocator       m_CommandAllocators[sBufferCount];
            PtrCmdQueue           m_CommandQueue;
            PtrDescHeap           m_DescriptorHeap;
            PtrPipeLineState      m_PSO;
            PtrResource           m_RenderTarget[sBufferCount];

			/* synchronization objects */
            UINT64                m_FenceValues[sBufferCount];
            UINT64                m_CurrentFrame;
            PtrFence              m_Fence;
            HANDLE                m_FenceEvent;


            DescriptorHeapWrapper m_RTVDescriptorHeap;
            Window *              m_Window;

            D3D12_VIEWPORT        m_ViewPort;
            RECT                  m_RectScissor;
        };
    }
}