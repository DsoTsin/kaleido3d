#pragma once

#include "IRHIDefs.h"

namespace rhi
{
	struct IGpuResource {};
	struct ISampler {};
	struct IVertexInputLayout {};
	struct IPipelineState;
	struct ICommandContext;
	struct IDevice;
	struct IDeviceAdapter {};
	struct IShaderCompiler;
	struct IShaderBytes;

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

		virtual ICommandContext*	NewCommandContext() = 0;
		virtual IGpuResource*		NewGpuResource(EGpuResourceType type) = 0;
		virtual ISampler*			NewSampler(const SamplerState&) = 0;
		virtual IPipelineState*		NewPipelineState() = 0;

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

	struct IPipelineState
	{
		virtual void SetShader(EShaderType, IShaderBytes*) = 0;
		virtual void SetRasterizerState(const RasterizerState&) = 0;
		virtual void SetBlendState(const BlendState&) = 0;
		virtual void SetDepthStencilState(const DepthStencilState&) = 0;
		virtual void SetVertexInputLayout(IVertexInputLayout *) = 0;
		virtual void SetSampler(ISampler*) = 0;
		virtual void Finalize() = 0;
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
		uint32 VertexCountPerInstance;
		uint32 InstanceCount;
		uint32 StartVertexLocation;
		uint32 StartInstanceLocation;
	};

	struct VertexBufferView {};
	struct IndexBufferView {};

	// Unthread-safe context, only for new APIs like D3D12,Vulkan..
	struct ICommandContext
	{
		virtual ~ICommandContext() {}
		virtual void Detach(IDevice *) = 0;
		virtual void CopyBuffer(IGpuResource& Dest, IGpuResource& Src) = 0;
		virtual void SetIndexBuffer(const IndexBufferView& IBView) = 0;
		virtual void SetVertexBuffer(uint32 Slot, const VertexBufferView& VBView) = 0;
		virtual void SetPipelineState(uint32 HashCode, IPipelineState*) = 0;
		virtual void SetViewport(const Viewport &) = 0;
		virtual void SetPrimitiveType(EPrimitiveType) = 0;
		virtual void DrawInstanced(DrawInstanceParam) = 0;
		virtual void DrawIndexedInstanced(DrawIndexedInstancedParam) = 0;
		virtual void Execute(bool Wait) = 0;
		virtual void Reset() = 0;
	};
}