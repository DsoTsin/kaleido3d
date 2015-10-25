#pragma once
#ifndef __IRHIDefs_h__
#define __IRHIDefs_h__

namespace rhi
{
	enum ECommandType
	{
		ECMD_Bundle,
		ECMD_Graphics,
		ECMD_Compute,
		ECommandTypeNum
	};

	enum EGpuResourceType
	{
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture2DMS,
		Texture2DMSArray,
		Texture3D,
		Buffer,
		ResourceTypeNum
	};

	enum EGpuMemViewType
	{
		SRV,
		UAV,
		CBV,
		Sampler,
		RTV,
		DSV,
		IBV,
		VBV,
		SOV,
		GpuViewTypeNum
	};

	enum EPrimitiveType
	{
		Points,
		Lines,
		Triangles,
		TriangleStrip,
		PrimTypeNum
	};

	enum EShaderType
	{
		ES_Fragment,
		ES_Vertex,
		ES_Geometry,
		ES_Hull,
		ES_Domain,
		ES_Compute,
		ShaderTypeNum
	};

	struct BlendState
	{
		enum EOperation
		{
			Add,
			Sub,
			BlendOpNum
		};
		enum EBlend
		{
			Zero,
			One,
			SrcColor,
			DestColor,
			SrcAlpha,
			DestAlpha,
			BlendTypeNum
		};

		bool		Enable;

		EBlend		Src;
		EBlend		Dest;
		EOperation	Op;

		EBlend		SrcBlendAlpha;
		EBlend		DestBlendAlpha;
		EOperation	BlendAlphaOp;

		BlendState()
		{
			Enable = false;
			Src = EBlend::One;
			Dest = EBlend::Zero;
			Op = EOperation::Add;

			SrcBlendAlpha = EBlend::One;
			DestBlendAlpha = EBlend::Zero;
			BlendAlphaOp = EOperation::Add;
		}
	};

	struct RasterizerState
	{
		enum EFillMode
		{
			WireFrame,
			Solid,
			FillModeNum
		};
		enum ECullMode
		{
			None,
			Front,
			Back,
			CullModeNum
		};

		EFillMode	FillMode;
		ECullMode	CullMode;
		bool		FrontCCW;
		int32		DepthBias;
		float		DepthBiasClamp;
		bool		DepthClipEnable;
		bool		MultiSampleEnable;

		RasterizerState()
		{
			FillMode = EFillMode::Solid;
			CullMode = ECullMode::Back;
			FrontCCW = false;
			DepthBias = 0;
			DepthBiasClamp = 0.0f;
			DepthClipEnable = true;
			MultiSampleEnable = false;
		}
	};

	struct DepthStencilState
	{
		enum EStencilOp
		{
			Keep,
			Zero,
			Replace,
			Invert,
			Increment,
			Decrement,
			StencilOpNum
		};

		enum EComparisonFunc
		{
			Never,
			Less,
			Equal,
			LessEqual,
			Greater,
			NotEqual,
			GreaterEqual,
			Always,
			ComparisonFuncNum
		};

		enum EDepthWriteMask
		{
			WriteZero,
			WriteAll,
			DepthWriteMaskNum
		};

		struct Op
		{
			EStencilOp StencilFailOp;
			EStencilOp DepthStencilFailOp;
			EStencilOp StencilPassOp;
			EComparisonFunc StencilFunc;

			Op()
			{
				StencilFailOp = EStencilOp::Keep;
				DepthStencilFailOp = EStencilOp::Keep;
				StencilPassOp = EStencilOp::Keep;
				StencilFunc = EComparisonFunc::Always;
			}
		};

		bool			DepthEnable;
		EDepthWriteMask DepthWriteMask;
		EComparisonFunc	DepthFunc;

		bool	StencilEnable;
		uint8	StencilReadMask;
		uint8	StencilWriteMask;
		Op		FrontFace;
		Op		BackFace;

		DepthStencilState()
		{
			DepthEnable = true;
			DepthWriteMask = EDepthWriteMask::WriteAll;
			DepthFunc = EComparisonFunc::Less;

			StencilEnable = false;
			StencilReadMask = 0xff;
			StencilWriteMask = 0xff;
		}
	};

	struct SamplerState
	{
		typedef DepthStencilState::EComparisonFunc EComparisonFunc;
		enum EFilterMethod
		{
			Point, // Nearest
			Linear,
			FilterMethodNum
		};
		enum EFilterReductionType
		{
			Standard,
			Comparison, // all three other filter should be linear
			Minimum,
			Maximum,
			FilterReductionTypeNum
		};
		enum ETextureAddressMode
		{
			Wrap,
			Mirror, // Repeat
			Clamp,
			Border,
			MirrorOnce,
			AddressModeNum
		};

		struct Filter
		{
			EFilterMethod			MinFilter;
			EFilterMethod			MagFilter;
			EFilterMethod			MipMapFilter;
			EFilterReductionType	ReductionType;
			Filter()
			{
				MinFilter = EFilterMethod::Linear;
				MagFilter = EFilterMethod::Linear;
				MipMapFilter = EFilterMethod::Linear;
				ReductionType = EFilterReductionType::Standard;
			}
		};

		Filter				Filter;
		ETextureAddressMode U, V, W;
		float				MipLODBias;
		uint32				MaxAnistropy;
		EComparisonFunc		ComparisonFunc;
		float				BorderColor[4];
		float				MinLOD;
		float				MaxLOD;

		SamplerState()
			: U(ETextureAddressMode::Wrap)
			, V(ETextureAddressMode::Wrap)
			, W(ETextureAddressMode::Wrap)
			, MipLODBias(0)
			, MaxAnistropy(16)
			, ComparisonFunc(EComparisonFunc::LessEqual)
			, MinLOD(0), MaxLOD(3.402823466e+38f)
		{
		}
	};

	struct Viewport
	{
		float Left;
		float Top;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	};

}

#endif