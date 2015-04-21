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

    private:
      /** First Step : create directx12 device **/
      HRESULT CreateDevice ();
      /** Second Step : create directx12 cmd allocator **/
      HRESULT CreateCmdAllocatorAndQueue ();
      /** Third Step : create swap chain **/
      HRESULT CreateSwapChain (Window * window);
      HRESULT CreateDescHeap ();
      HRESULT CreatePSO ();
      /** Create graphics command list*/
      HRESULT CreateGfxCmdList ();
      /** Create Render Target **/
      HRESULT CreateRenderTargets (Window * window);

      HRESULT CreateFence ();

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
      D3D12_VIEWPORT  mViewPort;
	  DescriptorHeapWrapper mRTVDescriptorHeap;

      Window * mWindow;

    };
  }
}