#pragma once

#include "IRHIDefs.h"

namespace rhi
{
	struct ICommandContext;
	struct IDevice;
	struct IShaderCompiler;
	struct IShaderBytes;

	struct IGpuResource 
	{
		uint64 Padding;
	};

	struct ISampler 
	{
		uint64 Padding;
	};
	
	struct IDeviceAdapter 
	{
        virtual IDevice * GetDevice() = 0;
	};

	/**
	* Vulkan has pipeline layout,
	* D3D12 is root signature.
	* PipelineLayout defines shaders impose constraints on table layout.
	*/
	struct IPipelineLayout
	{
		virtual void Create(ShaderParamLayout const &) = 0;
		virtual void Finalize(IDevice *) = 0;
	};

	extern IPipelineLayout * CreatePipelineLayout(ShaderParamLayout const &);

	struct ISyncPointFence
	{
	};

	struct IPipelineStateObject
	{
		virtual EPipelineType   GetType() = 0;
		virtual void			SetShader(EShaderType, IShaderBytes*) = 0;
		virtual void			SetLayout(IPipelineLayout*) = 0;
		virtual void			Finalize() = 0;
	};

	struct IGraphicsPipelineState
	{
		virtual void SetRasterizerState(const RasterizerState&) = 0;
		virtual void SetBlendState(const BlendState&) = 0;
		virtual void SetDepthStencilState(const DepthStencilState&) = 0;
		virtual void SetPrimitiveTopology(const EPrimitiveType) = 0;
		virtual void SetVertexInputLayout(rhi::VertexDeclaration *, uint32 Count) = 0;
		virtual void SetRenderTargetFormat(const RenderTargetFormat &) = 0;
		virtual void SetSampler(ISampler*) = 0;
	};

	struct IComputePipelineState
	{

	};

	// List all devices
	typedef void(*PFNEnumAllDevice)(IDeviceAdapter** &, uint32*);
	extern PFNEnumAllDevice EnumAllDeviceAdapter;

	struct IDevice
	{
		enum Result {
			DeviceNotFound,
			DeviceFound
		};

		virtual						~IDevice() {}
		virtual Result				Create(IDeviceAdapter *, bool withDebug) = 0;

		virtual ICommandContext*	NewCommandContext(ECommandType) = 0;
		virtual IGpuResource*		NewGpuResource(EGpuResourceType type) = 0;
		virtual ISampler*			NewSampler(const SamplerState&) = 0;
		virtual IPipelineStateObject*NewPipelineState(EPipelineType) = 0;
		virtual ISyncPointFence*	NewFence() = 0;
	};

	struct IShaderCompiler
	{
		enum ELangVersion
		{
			HLSL_5_0,
			HLSL_5_1,
			GLSL_4_3,
			METALSL_1_1,
			LangVersionNum
		};
		virtual IShaderBytes* CompileFromSource(ELangVersion, EShaderType, const char*) = 0;
	};

	struct IShaderBytes
	{
		virtual uint32		Length() = 0;
		virtual const void*	Bytes() = 0;
	};

	struct DrawIndexedInstancedParam
	{
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
    
    struct ICommandContext
    {
        virtual ~ICommandContext() {}
        
        virtual void Detach(IDevice *) = 0;
        virtual void CopyBuffer(IGpuResource& Dest, IGpuResource& Src) = 0;
        virtual void Execute(bool Wait) = 0;
        virtual void Reset() = 0;
    };

	struct IColorBuffer;
	struct IDepthBuffer;

	// Unthread-safe context, only for new APIs like D3D12,Vulkan..
    struct IGraphicsCommand
	{
        virtual ~IGraphicsCommand() {}
		virtual void ClearColorBuffer(IColorBuffer*) = 0;
		virtual void ClearDepthBuffer(IDepthBuffer*) = 0;
		virtual void SetRenderTargets(uint32 NumColorBuffer, IColorBuffer*, IDepthBuffer*, bool ReadOnlyDepth = false) = 0;
		virtual void SetScissorRects(uint32, const Rect*) = 0;
		virtual void SetViewport(const ViewportDesc &) = 0;
        virtual void SetIndexBuffer(const IndexBufferView& IBView) = 0;
		virtual void SetVertexBuffer(uint32 Slot, const VertexBufferView& VBView) = 0;
		virtual void SetPipelineState(uint32 HashCode, IPipelineStateObject*) = 0;
		virtual void SetPipelineLayout(IPipelineLayout *) = 0;
		virtual void SetPrimitiveType(EPrimitiveType) = 0;
		virtual void DrawInstanced(DrawInstanceParam) = 0;
		virtual void DrawIndexedInstanced(DrawIndexedInstancedParam) = 0;
	};
    
    struct IComputeCommand
    {
		virtual void SetPipelineLayout(IPipelineLayout *) = 0;
        virtual void Dispatch( uint32 GroupCountX, uint32 GroupCountY, uint32 GroupCountZ) = 0;
    };
}