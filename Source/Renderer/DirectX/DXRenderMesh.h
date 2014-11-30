#pragma once
#include <Interface/IRenderMesh.h>
#include <KTL/NonCopyable.hpp>
#include <Core/Mesh.h>

#include "DXCommon.h"
#include "DXShader.h"
#include "DirectXRenderer.h"

namespace k3d {

	class DXRenderMesh : public IRenderMesh /*, public NonCopyable*/ {
	public:

		DXRenderMesh();
		~DXRenderMesh() override;

		void Render() override;
		void Render(DxRef::DevContextPtr & device);
		
		void Init(DXDevice & device, SpMesh const & rawMesh, DXVertexShader & shader);
						
	private:

		DxRef::BufferPtr		pIndexBuffer;
		DxRef::BufferPtr		pVertexBuffer;
		DxRef::InputLayoutPtr	pVertexLayout;
			
		bool				m_MeshInitialized;
	};

}