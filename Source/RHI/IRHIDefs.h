#pragma once
#ifndef __IRHIDefs_h__
#define __IRHIDefs_h__

#include "Config/PlatformTypes.h"
#include <KTL/DynArray.hpp>
#include <Math/kGeometry.hpp>
//#include "Tools/ShaderGen/ShaderCompiler.h"

namespace rhi
{
	/**
	 * Global Enums
	 * Must Start with 'E'
	 **/

	enum ECommandType
	{
		ECMD_Bundle,
		ECMD_Graphics,
		ECMD_Compute,
		ECommandTypeNum
	};

	enum EPipelineType
	{
		EPSO_Compute,
		EPSO_Graphics
	};

	enum EPixelFormat
	{
		EPF_RGBA16Uint,
		EPF_RGBA32Float,
		EPF_RGBA8Unorm,
		EPF_RGBA8Unorm_sRGB,
		EPF_R11G11B10Float,
		EPF_D32Float,
		EPF_RGB32Float,
		PixelFormatNum
	};

	enum EVertexFormat
	{
		EVF_Float1x32,
		EVF_Float2x32,
		EVF_Float3x32,
		EVF_Float4x32,
		VertexFormatNum
	};

	enum EMultiSampleFlag
	{
		EMS_1x,
		EMS_2x,
		EMS_4x,
		EMS_8x,
		EMS_16x,
		EMS_32x,
		ENumMultiSampleFlag
	};

	enum EPrimitiveType
	{
		EPT_Points,
		EPT_Lines,
		EPT_Triangles,
		EPT_TriangleStrip,
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

	/** For pipeline layout declaration **/

	constexpr int MAX_SHADER_PARAM = 64;

	enum EShaderParam
	{
		ESP_UnAssigned,
		ESP_Texture,
		ESP_Buffer,
		ESP_32BitsConstant,
		ESP_Sampler,
		ShaderParamNum
	};

	/**
	 * @see GfxSetting
	 */
	struct RenderTargetFormat
	{
		RenderTargetFormat()
		{
			NumRTs = 1;
			RenderPixelFormats = new EPixelFormat{ EPF_RGBA8Unorm };
			DepthPixelFormat = EPF_RGBA8Unorm;
			MSAACount = 1;
		}
		uint32			NumRTs;
		EPixelFormat *	RenderPixelFormats;
		EPixelFormat 	DepthPixelFormat;
		uint32			MSAACount;
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
		EMultiSampleFlag MSFlag;

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
			DepthEnable = false;
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
	/*
	VK_IMAGE_LAYOUT_GENERAL / D3D12_RESOURCE_STATE_COMMON
	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL / D3D12_RESOURCE_STATE_RENDER_TARGET
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL / D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL / D3D12_RESOURCE_STATE_DEPTH_READ
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL / D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL / D3D12_RESOURCE_STATE_COPY_SOURCE
	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL / D3D12_RESOURCE_STATE_COPY_DEST
	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR / D3D12_RESOURCE_STATE_PRESENT
	*/
	enum EResourceState
	{
		ERS_Common,
		ERS_Present, // D3D12_RESOURCE_STATE_PRESENT/VK_IMAGE_LAYOUT_PRESENT_SRC_KHR?
		ERS_RenderTarget, // D3D12_RESOURCE_STATE_RENDER_TARGET/VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL?
		ERS_TransferDst,
		ERS_TransferSrc,
		ERS_RWDepthStencil,
		ERS_Unknown,
		ERS_Num
	};

	struct Rect
	{
		long  Left;
		long  Top;
		long  Right;
		long  Bottom;
	};

	/**
	 * @see RenderTargetFormat
	 * @members Width,Height,ColorFormat,BackbufferCount
	 */
	struct GfxSetting
	{
		GfxSetting()
			: Width(0), Height(0)
			, ColorFormat(EPixelFormat::EPF_RGBA8Unorm)
			, DepthStencilFormat(EPixelFormat::EPF_D32Float)
			, HasDepth(false)
			, BackBufferCount(2) // Default is 2
		{
		}

		/**
		 * @param width
		 * @param height
		 * @param format
		 * @param bufCount
		 */
		GfxSetting(uint32 width, uint32 height, EPixelFormat format, EPixelFormat dsFormat, bool hasDepth, uint32 bufCount)
			: Width(width), Height(height)
			, ColorFormat(format), DepthStencilFormat(dsFormat), HasDepth(hasDepth)
			, BackBufferCount(bufCount)
		{
		}

		uint32			Width;
		uint32			Height;
		EPixelFormat	ColorFormat;
		EPixelFormat	DepthStencilFormat;
		bool			HasDepth;
		uint32			BackBufferCount;
	};

	struct ViewportDesc
	{
		ViewportDesc()
			: ViewportDesc(0.f, 0.f)
		{
		}

		ViewportDesc(
			float width, float height,
			float left = 0.0f, float top = 0.0f,
			float minDepth = 0.0f, float maxDepth = 1.0f
			)
			: Left(left), Top(top), Width(width), Height(height), MinDepth(minDepth), MaxDepth(maxDepth)
		{
		}

		float Left;
		float Top;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	};
	/**
	 * EVertexFormat,
	 * Stride,
	 * AttributeID,
	 * Offset,
	 * BindID
	 */
	struct VertexDeclaration
	{
		EVertexFormat	Format;
		uint32			Stride;
		uint32			AttributeIndex;
		uint32			OffSet;
		uint32			BindID;
	};

	struct VertexBufferView
	{
		uint64 BufferLocation;
		uint32 SizeInBytes;
		uint32 StrideInBytes;
	};

	struct IndexBufferView
	{
		uint64 BufferLocation;
		uint32 SizeInBytes;
	};

	struct BufferRegion
	{
		uint64 SrcOffSet;
		uint64 DestOffSet;
		uint64 CopySize;
	};

	struct ShaderParamLayout
	{
		EShaderParam Location[MAX_SHADER_PARAM];
	};

	enum EGpuMemViewType
	{
		EGVT_Undefined,
		EGVT_VBV, // For VertexBuffer
		EGVT_IBV, // For IndexBuffer
		EGVT_CBV, // For ConstantBuffer,
		EGVT_SRV, // For Texture
		EGVT_UAV, // For Buffer
		EGVT_RTV,
		EGVT_DSV,
		EGVT_Sampler,
		EGVT_SOV,
		GpuViewTypeNum
	};

	enum EGpuResourceType
	{
		EGT_Texture1D,
		EGT_Texture1DArray,
		EGT_Texture2D,
		EGT_Texture2DArray,
		EGT_Texture2DMS,
		EGT_Texture2DMSArray,
		EGT_Texture3D,
		EGT_Buffer,
		ResourceTypeNum
	};

	enum EGpuResourceAccessFlag
	{
		EGRAF_Read = 0x1,
		EGRAF_Write = 0x1 << 1,
		EGRAF_ReadAndWrite = (EGRAF_Read | EGRAF_Write),
		EGRAF_HostVisible = 0x1 << 2,
		EGRAF_DeviceVisible = 0x1 << 3,
		EGRAF_HostCoherent = 0x1 << 4,
		EGRAF_HostCached = 0x1 << 5,
	};

	enum EGpuResourceCreationFlag
	{
		EGRCF_Dynamic = 0,
		EGRCF_Static = 1,
		EGRCF_TransferSrc = 2,
		EGRCF_TransferDst = 4
	};

	/**
	 * Format, Width, Height, Depth, MipLevel, Layers
	 */
	struct TextureDesc
	{
		EPixelFormat		Format;
		uint32				Width;
		uint32				Height;
		uint32				Depth;
		uint32				MipLevels;
		uint32				Layers;

		TextureDesc()
		{
			Format = EPixelFormat::EPF_RGBA8Unorm;
			Width = 0;
			Height = 0;
			Depth = 0;
			MipLevels = 0;
			Layers = 0;
		}

		bool				IsTex1D() const { return Depth == 1 && Layers == 1 && Height == 1; }
		bool				IsTex2D() const { return Depth == 1 && Layers == 1 && Height > 1; }
		bool				IsTex3D() const { return Depth > 1 && Layers == 1; }
	};

	struct ResourceDesc
	{
		EGpuResourceType Type;
		EGpuResourceAccessFlag Flag;
		EGpuResourceCreationFlag CreationFlag;
		EGpuMemViewType ViewType;
		union
		{
			TextureDesc TextureDesc;
			uint64 Size;
		};
		ResourceDesc() : Type(EGT_Buffer), Flag(EGRAF_ReadAndWrite), CreationFlag(EGRCF_Dynamic), ViewType(EGVT_SRV) { }
		ResourceDesc(EGpuResourceType t, EGpuResourceAccessFlag f, EGpuResourceCreationFlag cf, EGpuMemViewType viewType) :
			Type(t), Flag(f), CreationFlag(cf), ViewType(viewType) {}
	};

	typedef ::k3d::DynArray<uint32>				ShaderByteCode;
	typedef ::k3d::DynArray<VertexDeclaration>	VertexInputLayout;

	struct PipelineDesc
	{
		RasterizerState		Rasterizer;
		BlendState			Blend;
		DepthStencilState	DepthStencil;

		// Shaders
		ShaderByteCode		Shaders[ShaderTypeNum];
		// VertexAttributes
		VertexInputLayout	VertexLayout;
		// InputAssemblyState
		EPrimitiveType		PrimitiveTopology = rhi::EPT_Triangles;
	};

	/**
	 * IndexCountPerInstance
	 * InstanceCount
	 * StartLocation
	 * BaseVertexLocation
	 * StartInstanceLocation
	 */
	struct DrawIndexedInstancedParam
	{
		DrawIndexedInstancedParam(uint32 indexCountPerInst, uint32 instCount,
			uint32 startLoc = 0, uint32 baseVerLoc = 0, uint32 sInstLoc = 1)
			: IndexCountPerInstance(indexCountPerInst)
			, InstanceCount(instCount)
			, StartIndexLocation(startLoc)
			, BaseVertexLocation(baseVerLoc)
			, StartInstanceLocation(sInstLoc)
		{
		}
		uint32 IndexCountPerInstance;
		uint32 InstanceCount;
		uint32 StartIndexLocation;
		uint32 BaseVertexLocation;
		uint32 StartInstanceLocation;
	};

	struct DrawInstanceParam
	{
		DrawInstanceParam(uint32 vertexPerInst, uint32 instances, uint32 startLocation = 0, uint32 startInstLocation = 0)
			: VertexCountPerInstance(vertexPerInst)
			, InstanceCount(instances)
			, StartVertexLocation(startLocation)
			, StartInstanceLocation(startInstLocation)
		{}
		uint32 VertexCountPerInstance;
		uint32 InstanceCount;
		uint32 StartVertexLocation;
		uint32 StartInstanceLocation;
	};
}

#endif