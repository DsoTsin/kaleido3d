#include "Kaleido3D.h"
#include "DXRenderMesh.h"

#include <assert.h>

namespace k3d {

	using namespace std;
	using namespace DxRef;

	DXRenderMesh::DXRenderMesh()
	{
		pVertexBuffer = nullptr;
		pIndexBuffer = nullptr;
		m_MeshInitialized = false;
	}

	DXRenderMesh::~DXRenderMesh()
	{
	}

	void DXRenderMesh::Render()
	{
	}

	void DXRenderMesh::Render(DxRef::DevContextPtr & context)
	{
//		context->DrawIndexed();
	}

	void DXRenderMesh::Init(DXDevice & device, SpMesh const & rawMesh, DXVertexShader & vertexShader)
	{
		assert(rawMesh != nullptr);
		if (m_MeshInitialized)
			return;

		// Set the input layout
		VtxFormat fmt = rawMesh->GetVertexFormat();
		pVertexLayout = vertexShader.GetInputLayout(fmt);
		if (pVertexLayout)
		{
			device.ImmediateContext()->IASetInputLayout(pVertexLayout);
		}

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = Mesh::GetVertexByteWidth(fmt, rawMesh->GetVertexNum());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = rawMesh->GetVertexBuffer();

		device.Device()->CreateBuffer(&bd, &InitData, pVertexBuffer.GetInitReference());


		UINT offset = 0;
		uint32 _stride = Mesh::GetVertexStride(fmt);
		ID3D11Buffer* vbo = pVertexBuffer;
		device.ImmediateContext()->IASetVertexBuffers(0, 1, &vbo, &_stride, &offset);

		// Create IndexBuffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(uint32) * rawMesh->GetIndexNum();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = rawMesh->GetIndexBuffer();
		device.Device()->CreateBuffer(&bd, &InitData, pIndexBuffer.GetInitReference());

		device.ImmediateContext()->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set primitive topology
		if(rawMesh->GetPrimType() == PrimType::TRIANGLES)
			device.ImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_MeshInitialized = true;
		Debug::Out("DXRenderMesh", "Init succeed.");
	}



}