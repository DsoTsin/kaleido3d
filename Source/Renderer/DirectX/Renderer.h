#pragma once
#ifndef __DirectXRenderer_h__
#define __DirectXRenderer_h__

#include <KTL/NonCopyable.hpp>
#include <KTL/Singleton.hpp>
#include <KTL/RefCount.hpp>
#include <Interface/IRenderer.h>

#include <memory>

#include "DXCommon.h"
#include "Infrastructure/Shader.h"

namespace k3d {

    class Window;

    class DirectXRenderer : public IRenderer, 
		public Singleton<DirectXRenderer>, 
		public std::enable_shared_from_this< DirectXRenderer >
	{
      public:

        ~DirectXRenderer() override;

        void Initialize();

        void PrepareFrame() override;

        // traverse the scene tree and render the elements
        void DrawOneFrame() override;

        // do postprocessing and swap buffers
        void EndOneFrame() override;

        void DrawMesh( IRenderMesh *) override;
        void DrawMesh( IRenderMesh *, d3d12::Shader *);
        void DrawMesh( IRenderMesh *, d3d12::Shader *, Matrix4f const &matrix );

        void OnResize( int width, int height ) override;

        DirectXRenderer() = default;

      protected:

        void SwapBuffers();

        bool isInitialized;

      private:

        d3d12::PtrCmdAllocator	m_BundleAllocator;
		d3d12::PtrGfxCmdList	m_BundleCmdList;

        d3d12::PtrGfxCmdList	m_CmdList;
        d3d12::PtrRootSignature	m_RootSignature;

    };
}


#endif