#pragma once
#include <Kaleido3D.h>
#include <Renderer/DirectX/DXCommon.h>
#include <Renderer/DirectX/RHI.h>
#include <Renderer/DirectX/Resource.h>

using k3d::d3d12::IndexBuffer;
using k3d::d3d12::VertexBuffer;
using k3d::d3d12::PtrDevice;
using k3d::d3d12::PtrGfxCmdList;

struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};


class CubeMesh {
public:
	CubeMesh(ID3D12Device* device, PtrGfxCmdList list);
	~CubeMesh();

	void Render(PtrGfxCmdList list);

private:

	IndexBuffer*		m_IBO;
	VertexBuffer*		m_VBO;
};