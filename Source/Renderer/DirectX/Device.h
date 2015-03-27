#pragma once
#include <KTL/Singleton.hpp>
#include "DXCommon.h"
#include "Helper.h"

namespace k3d
{
  // Forward Declarations
  class Window;

  namespace d3d12
  {
    /**
      * RenderDevice
      * manage d3d12 device-related objects
      */
    class Device : public Singleton < Device >
    {
    public:
      ~Device ();

      void Init (Window *);
      void Destroy ();

      PtrDevice GetD3DDevice ();

	  void Clear();

      PtrSwapChain GetSwapChain ()
      {
        return mSwapChain;
      }

      PtrCmdQueue GetDefaultCommandQueue ()
      {
        return mCommandQueue;
      }

      PtrCmdAllocator GetDefaultCommandAllocator ()
      {
        return mCommandAllocator;
      }

      PtrGfxCmdList GetDefaultGraphicsCommandList ()
      {
        return mGfxCmdList;
      }

    protected:
      Device ();
      friend class Singleton<Device>;

      void WaitForCommandQueueFence ();
	  static DXGI_SWAP_CHAIN_DESC SwapChainDesc(HWND hWnd);
    
	private:
      HRESULT CreatePSO ();
      /** Create Render Target **/
      HRESULT CreateRenderTargets (Window * window, PtrGfxCmdList list);

    private:
	  static const uint32 sBufferCount = 4;
      PtrDevice  mDevice;
      PtrSwapChain mSwapChain;
      PtrDXGIDevice  mDXGIDevice;
      PtrCmdAllocator  mCommandAllocator;
      PtrCmdQueue  mCommandQueue;
      PtrDescHeap  mDescriptorHeap;
      PtrPipeLineState mPSO;
      PtrGfxCmdList  mGfxCmdList;
      PtrResource  mRenderTarget[sBufferCount];
      PtrFence mFence;
      HANDLE  mHandle;
	  DescriptorHeapWrapper mRTVDescriptorHeap;
      Window * mWindow;

	  D3D12_VIEWPORT mViewPort;
	  RECT mRectScissor;
    };
  }
}