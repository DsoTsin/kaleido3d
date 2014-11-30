#pragma once
#include "Engine/SceneManager.h"
#include "RHI/IRHI.h"
#include <memory>
#include <Core/Window.h>

namespace render
{
	enum EThread
	{
		PSOCompile,
		Render,
		CommandBlit
	};

	class K3D_API Renderable
	{
	public:
		Renderable(k3d::SObject & obj) {}

		virtual ~Renderable() {}
		virtual void Prepare() = 0;
		virtual void Render(rhi::ICommandContext *RHIContext) = 0;
	};

	class K3D_API Mesh
	{
	public:

		virtual			~Mesh() {}

		virtual void	Begin(rhi::ICommandContext * gfxCmd);
		virtual void	Render();

		virtual void	Upload();

		virtual void*	VertexBuffer() = 0;

	private:
		rhi::ICommandContext* m_OwnedCmd;
	};


	enum class RHIType
	{
		ERTMetal,
		ERTVulkan,
		ERTDirect3D12,
	};

	using PtrRHIDevice = std::shared_ptr<rhi::IDevice>;

	/**
	* Associated with RHI
	*/
	class K3D_API RenderContext
	{
	public:
		RenderContext();

		void Init(RHIType type = RHIType::ERTVulkan);

		void Attach(k3d::IWindow::Ptr hostWindow);

		void PrepareRenderingResource();

		void PreRender();

		void Render();

		void PostRender();

		void Destroy();

		~RenderContext();

		rhi::IDevice * GetDevice() { return m_pDevice; }

	protected:
		RHIType					m_RhiType;
		rhi::IDevice *			m_pDevice;
		rhi::IRenderViewport *	m_RenderVp;
	};
}