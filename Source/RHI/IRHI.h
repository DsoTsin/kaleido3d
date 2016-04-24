#pragma once

#include "IRHIDefs.h"
#include "Tools/ShaderGen/ShaderCompiler.h"

namespace rhi
{
	struct ICommandContext;
	struct IDevice;
	struct IShaderBytes;

	struct K3D_API IGpuResource
	{
		virtual			~IGpuResource() {}
		virtual void *	Map(uint64 start, uint64 size) = 0;
		virtual void	UnMap() = 0;
	};

	struct ISampler
	{
		virtual ~ISampler() {}
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

	extern K3D_API IPipelineLayout * CreatePipelineLayout(ShaderParamLayout const &);

	/**
	 * @see https://www.reddit.com/r/vulkan/comments/47tc3s/differences_between_vkfence_vkevent_and/?
	 * Fence is GPU to CPU syncs
	 * Set on GPU, Wait on CPU
	 */
	struct ISyncFence
	{
		virtual void Signal(int32 fenceVal) = 0;
		virtual void Reset() {}
		virtual void WaitFor(uint64 time) = 0;
		virtual ~ISyncFence() {}
	};

	/**
	 * Semaphore used for GPU to GPU syncs, 
	 * Specifically used to sync queue submissions (on the same or different queues),
	 * Set on GPU, Wait on GPU (inter-queue)
	 * Events: Set anywhere, Wait on GPU (intra-queue)
	 */
	struct ISemaphore;

	struct K3D_API IPipelineStateObject
	{
		virtual EPipelineType   GetType() = 0;
		virtual void			SetShader(EShaderType, ::k3d::IShaderCompilerOutput*) = 0;
		virtual void			SetLayout(IPipelineLayout*) = 0;
		virtual void			Finalize() = 0;
		virtual void			SetRasterizerState(const RasterizerState&) = 0;
		virtual void			SetBlendState(const BlendState&) = 0;
		virtual void			SetDepthStencilState(const DepthStencilState&) = 0;
		virtual void			SetPrimitiveTopology(const EPrimitiveType) = 0;
		virtual void			SetVertexInputLayout(rhi::VertexDeclaration const*, uint32 Count) = 0;
		virtual void			SetRenderTargetFormat(const RenderTargetFormat &) = 0;
		virtual void			SetSampler(ISampler*) = 0;
	};

	struct IDescriptor
	{

	};

	struct IDescriptorPool
	{
		virtual ~IDescriptorPool() {}
	};

	// List all devices
	typedef void(*PFNEnumAllDevice)(IDeviceAdapter** &, uint32*);
	extern PFNEnumAllDevice EnumAllDeviceAdapter;

	struct IRenderViewport;
	struct K3D_API IDevice
	{
		enum Result {
			DeviceNotFound,
			DeviceFound
		};

		virtual								~IDevice() {}
		virtual Result						Create(IDeviceAdapter *, bool withDebug) = 0;

		virtual ICommandContext*			NewCommandContext(ECommandType) = 0;
		virtual IGpuResource*				NewGpuResource(ResourceDesc const&,uint64) = 0;
		virtual ISampler*					NewSampler(const SamplerState&) = 0;
		virtual IPipelineStateObject*		NewPipelineState(EPipelineType) = 0;
		virtual ISyncFence*					NewFence() = 0;
		virtual IDescriptorPool*			NewDescriptorPool() = 0;
		virtual IRenderViewport *			NewRenderViewport(void * winHandle, uint32 width, uint32 height) = 0;
		virtual ::k3d::IShaderCompiler *	NewShaderCompiler() = 0;
	};

	struct K3D_API IRenderViewport
	{
		virtual						~IRenderViewport() {}

		virtual bool				InitViewport(
										void *windowHandle, 
										IDevice * pDevice, 
										uint32 width, uint32 height,
										EPixelFormat rtFmt
									) = 0;

		virtual void				PrepareNextFrame() {}

		/**
		 * @param	vSync	true to synchronise.
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool				Present(bool vSync) = 0;
	};

	/*
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
	*/
	struct IShaderBytes
	{
		virtual uint32		Length() = 0;
		virtual const void*	Bytes() = 0;
	};

	struct IColorBuffer;
	struct IDepthBuffer;

	struct K3D_API ICommandContext
	{
		virtual ~ICommandContext() {}

		virtual void Detach(IDevice *) = 0;
		virtual void CopyBuffer(IGpuResource& Dest, IGpuResource& Src) = 0;
		virtual void Execute(bool Wait) = 0;
		virtual void Reset() = 0;

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

		virtual void Dispatch(uint32 GroupCountX, uint32 GroupCountY, uint32 GroupCountZ) = 0;
	};

}