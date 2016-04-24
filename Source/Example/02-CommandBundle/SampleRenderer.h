#pragma once

#include <KTL/NonCopyable.hpp>
#include <KTL/Singleton.hpp>
#include <Interface/IRenderer.h>
#include <Core/TaskWrapper.h>

#include <memory>

#include <Core>
#include <Renderer_DirectX>
#include "Renderer/DirectX/Resource.h"

struct  ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

class CubeMesh;

namespace k3d {

	class Window;

	namespace d3d12 {

		class Renderable;

		class SampleRenderer
		{
		public:
			~SampleRenderer();
			void Initialize();
			void PrepareFrame();
			void DrawOneFrame();
			void EndOneFrame();
			void Render(Renderable *);
			void OnResize(int width, int height);
			SampleRenderer() = default;

		protected:
			void SwapBuffers();
			bool isInitialized;

		private:
			PtrCmdAllocator	    m_BundleAllocator;
			PtrGfxCmdList	    m_BundleCmdList;
			PtrGfxCmdList	    m_CmdList;
			PtrRootSignature	m_RootSignature;
			PtrPipeLineState    m_PipeLineState;

			CommandContext*		m_CmdCtx;

			CubeMesh*			m_CubeMesh;

			ModelViewProjectionConstantBuffer m_MVP;
			void *				m_ConstantBuffer;
			PtrDescHeap			m_CBVHeap;

			::Dispatch::WorkQueue * m_RenderQueue;
		};

		class DeviceManager;

		extern std::shared_ptr<DeviceManager>	gD3DDevice;
	}
}
