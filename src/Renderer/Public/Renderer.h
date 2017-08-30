#pragma once
#include "RendererMinimal.h"

namespace render
{
	enum EThread
	{
		PSOCompile,
		Render,
		CommandBlit
	};

	class RenderContext;

	class K3D_CORE_API Renderable
	{
	public:
		Renderable();
		virtual ~Renderable();
		virtual void Prepare() = 0;
	protected:
		bool			m_IsVisible;
		kMath::Vec4f	m_Position;
		kMath::Mat4f	m_ModelMatrix;
	};

	class K3D_CORE_API RenderableMesh : public Renderable
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

	using PtrRHIDevice = k3d::SharedPtr<NGFXDevice>;

	/**
	* Associated with RHI
	*/
	class K3D_CORE_API RenderContext
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

		NGFXDeviceRef GetDevice() { return m_pDevice; }
		
	protected:
		RHIType					m_RhiType;
		NGFXDeviceRef			m_pDevice;
		uint32 					m_Width = 0;
		uint32 					m_Height = 0;
	};
}