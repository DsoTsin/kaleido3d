#include "DXRenderMesh.h"

#include <assert.h>

namespace k3d {

	using namespace std;

	DXRenderMesh::DXRenderMesh()
	{
		pVertexBuffer = nullptr;
		pIndexBuffer = nullptr;
	}

	DXRenderMesh::~DXRenderMesh()
	{
	}

	void DXRenderMesh::Render()
	{
	}

	void DXRenderMesh::Render(DXDevice & device, SpMesh const & rawMesh, SpShader const & shader)
	{
		assert(rawMesh != nullptr);
		const shared_ptr<DXVertexShader> _vertexShader = static_pointer_cast<DXVertexShader>(shader);
		
		std::vector<D3D11_INPUT_ELEMENT_DESC>	_inputDesc;
		ID3D11InputLayout *						_inputLayout = nullptr;

		UINT _byteWidth = 0;
		UINT _stride	= 0;
		switch (rawMesh->GetVertexFormat()) {
		case POS3_F32_NOR3_F32_UV2_F32:
			_inputDesc = {
				{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOOD0",	0, DXGI_FORMAT_R32G32_FLOAT,	0,	24,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			_byteWidth = sizeof(Vertex3F3F2F) * rawMesh->GetVertexNum();
			_stride = sizeof(Vertex3F3F2F);
			break;
		case POS3_F32_NOR3_F32:
			_inputDesc = {
				{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			_byteWidth = sizeof(Vertex3F3F) * rawMesh->GetVertexNum();
			_stride = sizeof(Vertex3F3F);
			break;
		}
		// Create the input layout
		HRESULT hr = device.Device()->CreateInputLayout(&_inputDesc[0], (UINT)_inputDesc.size(), _vertexShader->GetBlob().Data(), _vertexShader->GetBlob().Size(), &_inputLayout);

		if (FAILED(hr)) {
			return;
		}
		// Set the input layout
		device.ImmediateContext()->IASetInputLayout(_inputLayout);
		
		// Then 
		pVertexLayout = make_shared<ID3D11InputLayout*>(_inputLayout);
		
		// Create VertexBuffer
		ID3D11Buffer * _vertexBuffer = nullptr;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = _byteWidth;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = rawMesh->GetVertexBuffer();

		device.Device()->CreateBuffer(&bd, &InitData, &_vertexBuffer);

		// Then
		pVertexBuffer = make_shared<ID3D11Buffer*>(_vertexBuffer);

		UINT offset = 0;
		device.ImmediateContext()->IASetVertexBuffers(0, 1, &_vertexBuffer, &_stride, &offset);

		// Create IndexBuffer
		ID3D11Buffer * _indexBuffer = nullptr;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(uint32) * rawMesh->GetIndexNum();        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = rawMesh->GetIndexBuffer();
		device.Device()->CreateBuffer(&bd, &InitData, &_indexBuffer);

		device.ImmediateContext()->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set primitive topology
		device.ImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}



}