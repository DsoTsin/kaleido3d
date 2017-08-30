#include "ngfx.enums.hpp"

namespace ngfx {
    struct DepthStencilOp {
        StencilOperation    stencilFailOp;
        StencilOperation    depthStencilFailOp;
        StencilOperation    stencilPassOp;
        ComparisonFunction  stencilFunc;
        uint32              compareMask;
        uint32              writeMask;
        uint32              reference;
    };
    struct DepthStencilState {
        bool                depthTest;
        DepthWriteMask      depthWriteMask;
        ComparisonFunction  depthFunction;
        bool                stencilTest;
        DepthStencilOp      front;
        DepthStencilOp      back;
    };
    struct RasterizerState {
        FillMode            fillMode;
        CullMode            cullMode;
        bool                frontCCW;
        bool                depthClipEnable;
        float               depthBiasClamp;
        float               depthBiasSlope;
        float               depthBias;
        MultisampleFlags    multisample;
    };
    struct RenderTargetBlendState {
        bool                blendEnable;
        BlendFactor         srcColor;
        BlendFactor         destColor;
        BlendOperation      colorOp;
        BlendFactor         srcAlpha;
        BlendFactor         destAlpha;
        BlendOperation      alphaOp;
        uint32              colorWriteMask;
    };
    struct BlendState {
       array<RenderTargetBlendState, 8> renderTargets;
    };
	struct VertexLayout {
		VertexInputRate		inputRate;
		uint32				stride;
	};
	struct VertexAttribute {
		PixelFormat format;
		uint32 offset;
		uint32 slot;
	};
    struct VertexInputState {
		array<VertexAttribute>	attributes;
		array<VertexLayout>		layouts;
    };
	[[bitmask("true")]]
	enum BufferUsage {
        VertexBuffer,
        IndexBuffer,
        UniformBuffer,
		UnorderedAccess,
		AccelerationStructure
	};
	[[bitmask("true")]]
	enum TextureUsage {
        ShaderResource,
        RenderTarget,
        DepthStencil
	};
	struct BufferDesc {
		BufferUsage usages;
		uint32		size;
		uint64		deviceMask;
	};
	struct TextureDesc {
		TextureUsage usages;
		PixelFormat format;
		uint32		width;
		uint32		height;
		uint32		depth;
		uint32		layers;
		uint32		mipLevels;
		uint64		deviceMask;
	};
	struct RaytracingAABBs
	{
		uint32 count;
	};
	struct RaytracingTriangles
	{
		uint32 count;
	};
	struct RaytracingGeometryData
	{
		RaytracingAABBs aabbs;
		RaytracingTriangles triangles;
	};
	struct RaytracingGeometryDesc
	{
		RaytracingGeometryType type;
		RaytracingGeometryFlags flag;
		RaytracingGeometryData data;
	};
	struct RaytracingASDesc {
		AccelerationStructureType type;
		AccelerationStructureBuildFlag flag;
		uint32 instanceCount;
		uint32 geometryCount;
		const RaytracingGeometryDesc* pGeometries;
	};
	struct SamplerDesc {
		FilterMode minFilter;
		FilterMode magFilter;
		FilterMode mipMapFilter;
		SamplerAddressMode u;
		SamplerAddressMode v;
		SamplerAddressMode w;
		ComparisonFunction comparisonFunction;
		uint32 maxAnistropy;
		float minLodBias;
		float minLod;
		float maxLod;
	};
}