#pragma once

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

#include <KTL/RefCount.hpp>

namespace k3d
{
	using Vector3f = DirectX::XMFLOAT3;
	using Vector4f = DirectX::XMVECTOR;
	using Matrix4f = DirectX::XMMATRIX;

	namespace DxRef
	{
		using DevPtr			= Ref<ID3D11Device>;
		using DevContextPtr		= Ref<ID3D11DeviceContext>;
		using CmdListPtr		= Ref<ID3D11CommandList>;

		using VertexShaderPtr	= Ref<ID3D11VertexShader>;
		using PixelShaderPtr	= Ref<ID3D11PixelShader>;
		using ComputeShaderPtr	= Ref<ID3D11ComputeShader>;

		using InputLayoutPtr	= Ref<ID3D11InputLayout>;

		using BufferPtr			= Ref<ID3D11Buffer>;
		using Tex2DPtr			= Ref<ID3D11Texture2D>;
		using Tex3DPtr			= Ref<ID3D11Texture3D>;
	}

}