#pragma once
#include <KTL/Singleton.hpp>
#include "DXCommon.h"

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

      PtrDevice GetDevice ();

      Device ();

    protected:
      void WaitForCommandQueueFence ();

    private:
      /** First Step : create directx12 device **/
      HRESULT CreateDevice ();
      /** Second Step : create directx12 cmd allocator **/
      HRESULT CreateCmdAllocator ();
      /** Third Step : create swap chain **/
      HRESULT CreateSwapChain (Window * window);
      /****/
      HRESULT CreateDescHeap ();
      /** In Direct3D 12, required pipeline state is attached to a command list via a pipeline state object (PSO) **/
      HRESULT CreatePSO ();
      /**create graphics command list*/
      HRESULT CreateGfxCmdList ();
      /** Create Render Target **/
      HRESULT CreateRTV ();

      HRESULT CreateFence ();

      friend class DirectXRenderer;

    private:

      PtrDevice  mDevice;
      PtrSwapChain mSwapChain;
      PtrDXGIDevice  mDXGIDevice;
      PtrCmdAllocator  mCommandAllocator;
      PtrCmdQueue  mCommandQueue;
      PtrDescHeap  mDescriptorHeap;
      PtrPipeLineState mPSO;
      PtrGfxCmdList  mGfxCmdList;
      PtrResource  mRenderTarget[2];
      PtrFence mFence;
      HANDLE  mHandle;
      D3D12_VIEWPORT  mViewPort;
      D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetView[2];
    };
  }
}