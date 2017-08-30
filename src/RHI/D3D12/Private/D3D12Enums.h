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

	D3D12_PRIMITIVE_TOPOLOGY_TYPE g_PrimType[rhi::EPrimitiveType::PrimTypeNum] =
	{
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	};

	D3D12_BLEND_OP g_BlendOps[rhi::BlendState::BlendOpNum] = {
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_OP_SUBTRACT,
	};

	D3D12_BLEND g_Blend[rhi::BlendState::BlendTypeNum] = {
		D3D12_BLEND_ZERO,
		D3D12_BLEND_ONE,
		D3D12_BLEND_SRC_COLOR,
		D3D12_BLEND_DEST_COLOR,
		D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND_DEST_ALPHA
	};

	D3D12_FILL_MODE g_FillMode[rhi::RasterizerState::EFillMode::FillModeNum] = {
		D3D12_FILL_MODE_WIREFRAME,
		D3D12_FILL_MODE_SOLID
	};

	D3D12_CULL_MODE g_CullMode[rhi::RasterizerState::CullModeNum] = {
		D3D12_CULL_MODE_NONE,
		D3D12_CULL_MODE_FRONT,
		D3D12_CULL_MODE_BACK
	};

	D3D12_STENCIL_OP g_StencilOp[rhi::DepthStencilState::StencilOpNum] = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_ZERO,
		D3D12_STENCIL_OP_REPLACE,
		D3D12_STENCIL_OP_INVERT,
		D3D12_STENCIL_OP_INCR,
		D3D12_STENCIL_OP_DECR
	};

	D3D12_COMPARISON_FUNC g_ComparisonFunc[rhi::DepthStencilState::ComparisonFuncNum] = {
		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_COMPARISON_FUNC_LESS,
		D3D12_COMPARISON_FUNC_EQUAL,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER,
		D3D12_COMPARISON_FUNC_NOT_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		D3D12_COMPARISON_FUNC_ALWAYS
	};

	D3D12_DEPTH_WRITE_MASK g_DepthWriteMask[rhi::DepthStencilState::DepthWriteMaskNum] = {
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_DEPTH_WRITE_MASK_ALL
	};

	/*
        EPF_RGBA16Uint,
        EPF_RGBA32Float,
        EPF_RGBA8Unorm,
        EPF_RGBA8Unorm_sRGB,
	*/

	DXGI_FORMAT g_DXGIFormatTable[rhi::EPixelFormat::PixelFormatNum] = {
		DXGI_FORMAT_R16G16B16A16_UINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_R11G11B10_FLOAT,
		DXGI_FORMAT_D32_FLOAT
	};

	/*
		EVF_Float2x32,
		EVF_Float3x32,
		EVF_Float4x32,
		VertexFormatNum
	*/

	DXGI_FORMAT g_VertexFormatTable[rhi::EVertexFormat::VertexFormatNum] = {
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};

	inline void RHIBlendDesc(D3D12_BLEND_DESC & Desc, rhi::BlendState const & BState)
	{
		D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc =
		{
			BState.Enable ? TRUE : FALSE, FALSE,
			g_Blend[BState.Src] ,g_Blend[BState.Dest], g_BlendOps[BState.Op],
			g_Blend[BState.SrcAlpha], g_Blend[BState.DestAlpha], g_BlendOps[BState.BlendAlphaOp],
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};


		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			Desc.RenderTarget[i] = RenderTargetBlendDesc;

		Desc.AlphaToCoverageEnable			= FALSE;
		Desc.IndependentBlendEnable			= FALSE;
	}

	inline void RHIRasterizerDesc(D3D12_RASTERIZER_DESC & Desc, rhi::RasterizerState const & RasterState)
	{
		Desc.FillMode					= g_FillMode[RasterState.FillMode];
		Desc.CullMode					= g_CullMode[RasterState.CullMode];
		Desc.FrontCounterClockwise		= RasterState.FrontCCW ? TRUE : FALSE;
		Desc.DepthBias					= RasterState.DepthBias;
		Desc.DepthBiasClamp				= RasterState.DepthBiasClamp;
		Desc.DepthClipEnable			= RasterState.DepthClipEnable ? TRUE : FALSE;
		Desc.MultisampleEnable			= RasterState.MultiSampleEnable ? TRUE : FALSE;
		Desc.AntialiasedLineEnable		= FALSE;
		Desc.SlopeScaledDepthBias		= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		Desc.ForcedSampleCount			= 0; // can only be 0,1,4,8
		Desc.ConservativeRaster			= D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	inline void RHIDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC & Desc, rhi::DepthStencilState const& DState)
	{
		Desc.DepthEnable					= DState.DepthEnable ? TRUE : FALSE;
		Desc.DepthWriteMask					= g_DepthWriteMask	[	DState.DepthWriteMask	];
		Desc.DepthFunc						= g_ComparisonFunc	[	DState.DepthFunc		];

		Desc.StencilEnable					= DState.StencilEnable ? TRUE : FALSE;
		Desc.StencilReadMask				= DState.StencilReadMask;
		Desc.StencilWriteMask				= DState.StencilWriteMask;

		Desc.FrontFace.StencilFailOp		= g_StencilOp		[	DState.FrontFace.StencilFailOp		];
		Desc.FrontFace.StencilDepthFailOp	= g_StencilOp		[	DState.FrontFace.DepthStencilFailOp	];
		Desc.FrontFace.StencilPassOp		= g_StencilOp		[	DState.FrontFace.StencilPassOp		];
		Desc.FrontFace.StencilFunc			= g_ComparisonFunc	[	DState.FrontFace.StencilFunc		];

		Desc.BackFace.StencilFailOp			= g_StencilOp		[	DState.BackFace.StencilFailOp		];
		Desc.BackFace.StencilDepthFailOp	= g_StencilOp		[	DState.BackFace.DepthStencilFailOp	];
		Desc.BackFace.StencilPassOp			= g_StencilOp		[	DState.BackFace.StencilPassOp		];
		Desc.BackFace.StencilFunc			= g_ComparisonFunc	[	DState.BackFace.StencilFunc			];
	}

	inline D3D12_INPUT_ELEMENT_DESC RHIInputElementDesc(rhi::VertexDeclaration const & Dec, LPCSTR SemanticName )
	{
		D3D12_INPUT_ELEMENT_DESC Desc;
		Desc.SemanticName = SemanticName;
		Desc.SemanticIndex = Dec.AttributeIndex;
		Desc.InputSlot = 0;
		Desc.AlignedByteOffset = Dec.OffSet;
		Desc.InstanceDataStepRate = 0;
		Desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	}
}


NS_K3D_D3D12_END