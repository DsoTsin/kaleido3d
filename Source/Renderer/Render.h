#pragma once
#include "Engine/SceneManager.h"
#include "RHI/IRHI.h"

namespace render
{
	enum EThread
	{
		PSOCompile,
		Render,
		CommandBlit
	};

	class Renderable
	{
	public:
		Renderable(k3d::SObject & obj) {}

		virtual ~Renderable() {}
		virtual void Prepare() = 0;
		virtual void Render(rhi::ICommandContext *RHIContext) = 0;
	};

	class Renderer 
	{
	public:
		Renderer();

		virtual ~Renderer();

		virtual void BeginScene()
		{
			for (auto & r : m_CurrentRenderables)
			{
				r->Prepare();
			}
		}
		
	protected:
		std::vector<Renderable*> m_CurrentRenderables;
	};
}