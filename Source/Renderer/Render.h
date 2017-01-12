#pragma once
#include "Engine/SceneManager.h"
#include "RHI/IRHI.h"
#include <Math/kMath.hpp>
#include <memory>
#include <Core/Window.h>
#include <Core/MeshData.h>

namespace render
{
	enum EThread
	{
		PSOCompile,
		Render,
		CommandBlit
	};

	class RenderContext;

	class K3D_API Renderable
	{
	public:
		Renderable();
		virtual ~Renderable();
		virtual void Prepare() = 0;
		virtual void Render(rhi::ICommandContext *RHIContext) = 0;

	protected:
		bool			m_IsVisible;
		kMath::Vec4f	m_Position;
		kMath::Mat4f	m_ModelMatrix;
	};

	class K3D_API RenderableMesh : public ::k3d::MeshData, public Renderable
	{
	public:
		RenderableMesh();
		~RenderableMesh() override;

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

		void Init(RHIType type = RHIType::ERTVulkan, uint32 w = 0, uint32 h = 0);

		void Attach(k3d::IWindow::Ptr hostWindow);

		void PrepareRenderingResource();

		void PreRender();

		void Render();

		void PostRender();

		void Destroy();

		~RenderContext();

		rhi::DeviceRef GetDevice() { return m_pDevice; }
		rhi::RenderViewportRef GetViewport() { return m_RenderVp; }

	protected:
		RHIType					m_RhiType;
		rhi::DeviceRef			m_pDevice;
		rhi::RenderViewportRef	m_RenderVp;
		uint32 					m_Width = 0;
		uint32 					m_Height = 0;
	};
}