#pragma once
#include <Interface/IRenderMesh.h>
#include <Core/Mesh.h>

#include <memory>
#include <d3d11.h>

#include "DXShader.h"
#include "DirectXRenderer.h"

namespace k3d {

	class DXRenderMesh : public IRenderMesh {
	public:

		DXRenderMesh();

		~DXRenderMesh() override;

		void Render() override;

		typedef std::shared_ptr<ID3D11Buffer*>			SpBuffer;
		typedef std::shared_ptr<ID3D11InputLayout*>		SpInputLayout;

	private:

		void Render(DXDevice & device, SpMesh const & rawMesh, SpShader const & shader);

		DXRenderMesh(const DXRenderMesh &) = delete;
		DXRenderMesh & operator=(const DXRenderMesh &) = delete;
		
	private:

		SpInputLayout	   pVertexLayout;
		SpBuffer           pVertexBuffer;
		SpBuffer           pIndexBuffer; 
	};

}