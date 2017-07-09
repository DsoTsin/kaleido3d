#include "CubeMesh.h"

unsigned short cubeIndices[] =
{
	0,2,1, // -x
	1,2,3,

	4,5,6, // +x
	5,7,6,

	0,1,5, // -y
	0,5,4,

	2,6,7, // +y
	2,7,3,

	0,4,6, // -z
	0,6,2,

	1,3,7, // +z
	1,7,5,
};

VertexPositionColor cubeVertices[] =
{
	{ DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) },
	{ DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) },
	{ DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) },
	{ DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) },
	{ DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) },
};

CubeMesh::CubeMesh(ID3D12Device* d3dDevice, PtrGfxCmdList list)
	: m_IBO (nullptr)
	, m_VBO (nullptr)
{
	m_IBO = new IndexBuffer("Cube Indices", d3dDevice, DXGI_FORMAT_R16_UINT, sizeof(cubeIndices));
	m_IBO->Upload(list, cubeIndices);

	m_VBO = new VertexBuffer("Cube Vertices", d3dDevice, sizeof(VertexPositionColor), sizeof(cubeVertices));
	m_VBO->Upload(list, cubeVertices);
}

CubeMesh::~CubeMesh()
{
	if (m_IBO) {
		delete m_IBO;
		m_IBO = nullptr;
	}

	if (m_VBO) {
		delete m_VBO;
		m_VBO = nullptr;
	}
}

void CubeMesh::Render(PtrGfxCmdList list)
{
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->IASetVertexBuffers(0, 1, &(m_VBO->GetView()));
	list->IASetIndexBuffer(&(m_IBO->GetView()));
	list->DrawIndexedInstanced(36, 1, 0, 0, 0);
}
