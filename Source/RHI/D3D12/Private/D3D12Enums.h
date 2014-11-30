#pragma once

NS_K3D_D3D12_BEGIN

namespace
{
	D3D12_SHADER_VERSION_TYPE g_ShaderType[rhi::EShaderType::ShaderTypeNum] =
	{
		D3D12_SHVER_PIXEL_SHADER,
		D3D12_SHVER_VERTEX_SHADER,
		D3D12_SHVER_GEOMETRY_SHADER,
		D3D12_SHVER_HULL_SHADER,
		D3D12_SHVER_DOMAIN_SHADER,
		D3D12_SHVER_COMPUTE_SHADER
	};

	D3D12_PRIMITIVE_TOPOLOGY g_PrimitiveTopology[rhi::EPrimitiveType::PrimTypeNum] =
	{
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	};

}


NS_K3D_D3D12_END