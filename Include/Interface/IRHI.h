#pragma once

#include "ICrossShaderCompiler.h"

namespace rhi
{
	struct ICommandContext;
	typedef ::k3d::SharedPtr<ICommandContext> 	CommandContextRef;
	struct IDevice;
	typedef ::k3d::SharedPtr<IDevice> 			DeviceRef;
	struct IGpuResource;
	typedef ::k3d::SharedPtr<IGpuResource>		GpuResourceRef;
	struct ITexture;
	typedef ::k3d::SharedPtr<ITexture>			TextureRef;
	struct IShaderResourceView;
	typedef ::k3d::SharedPtr<IShaderResourceView>	ShaderResourceViewRef;
	struct IPipelineStateObject;
	typedef ::k3d::SharedPtr<IPipelineStateObject> PipelineStateObjectRef;
	struct IRenderViewport;
	typedef ::k3d::SharedPtr<IRenderViewport>	RenderViewportRef;
	struct IShaderBytes;
	struct ISampler;
	typedef ::k3d::SharedPtr<ISampler>			SamplerRef;
	typedef const SamplerRef					SamplerCRef;
	struct IDescriptor;
	typedef ::k3d::SharedPtr<IDescriptor>		DescriptorRef;
	struct IRenderTarget;
	typedef ::k3d::SharedPtr<IRenderTarget>	RenderTargetRef;

	/**
	 * Semaphore used for GPU to GPU syncs,
	 * Specifically used to sync queue submissions (on the same or different queues),
	 * Set on GPU, Wait on GPU (inter-queue)
	 * Events: Set anywhere, Wait on GPU (intra-queue)
	 */
	struct ISemaphore;

	/**
	 * @see https://www.reddit.com/r/vulkan/comments/47tc3s/differences_between_vkfence_vkevent_and/?
	 * Fence is GPU to CPU syncs
	 * Set on GPU, Wait on CPU
	 */
	struct ISyncFence;
	typedef ::k3d::SharedPtr<ISyncFence> SyncFenceRef;

	/**
	* Vulkan has pipeline layout,
	* D3D12 is root signature.
	* PipelineLayout defines shaders impose constraints on table layout.
	*/
	struct IPipelineLayout;
	typedef ::k3d::SharedPtr<IPipelineLayout>	PipelineLayoutRef;

	struct K3D_API IGpuResource
	{
		virtual					~IGpuResource() {}
		virtual void *				Map(uint64 start, uint64 size) = 0;
		virtual void				UnMap() = 0;

		virtual uint64				GetResourceLocation() const	{ return 0; }
		virtual ResourceDesc		GetResourceDesc() const = 0;
		
		/**
		 * Vulkan: texture uses image layout as resource state
		 * D3D12: used for transition, maybe used as ShaderVisiblity determination in STATIC_SAMPLER and descriptor table 
		 */
		virtual EResourceState		GetUsageState() const		{ return ERS_Unknown; }
		virtual EGpuResourceType	GetResourceType() const		{ return ResourceTypeNum; }
		virtual uint64				GetResourceSize() const = 0;
	};

	struct ISampler
	{
		virtual SamplerState GetSamplerDesc() const = 0;
		virtual ~ISampler() {}
	};

	struct ITexture;

	struct IShaderResourceView
	{
		virtual GpuResourceRef		GetResource() const = 0;
		virtual ResourceViewDesc	GetDesc() const = 0;
	};

	struct ITexture : virtual public IGpuResource
	{
		virtual						~ITexture() {}
		virtual SamplerCRef			GetSampler() const = 0;
		virtual void					BindSampler(SamplerRef) = 0;
		virtual void					SetResourceView(ShaderResourceViewRef) = 0;
		virtual ShaderResourceViewRef 	GetResourceView() const = 0;
	};

	struct IDescriptor
	{
		virtual void Update(uint32 bindSet, GpuResourceRef) = 0;
		virtual ~IDescriptor() {}
	};

	struct IDeviceAdapter
	{
		virtual DeviceRef GetDevice() = 0;
	};

	/**
	* Vulkan has pipeline layout,
	* D3D12 is root signature.
	* PipelineLayout defines shaders impose constraints on table layout.
	*/
	struct IPipelineLayout
	{
		virtual DescriptorRef GetDescriptorSet() const = 0;
		virtual ~IPipelineLayout() {}
	};
	
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

	struct RenderTargetLayout
	{
		struct Attachment
		{
			int 			Binding = -1;
			EPixelFormat 	Format = EPF_RGBA8Unorm;
		};
		::k3d::DynArray<Attachment> Attachments;
		bool 						HasDepthStencil;
		EPixelFormat 				DepthStencilFormat;
	};

	struct IRenderTarget
	{
		virtual ~IRenderTarget() {}
		virtual void			SetClearColor(kMath::Vec4f clrColor) = 0;
		virtual void			SetClearDepthStencil(float depth, uint32 stencil) = 0;
		virtual GpuResourceRef	GetBackBuffer() = 0;
	};

	struct PipelineDesc
	{
		RasterizerState		Rasterizer;
		BlendState			Blend;
		DepthStencilState	DepthStencil;

		// Shaders
		ShaderBundle	   Shaders[ShaderTypeNum];
		// VertexAttributes
		VertexInputLayout	VertexLayout;
		// InputAssemblyState
		EPrimitiveType		PrimitiveTopology = rhi::EPT_Triangles;
	};

	struct K3D_API IPipelineStateObject
	{
		virtual				~IPipelineStateObject() {}
		virtual EPipelineType  GetType() = 0;
		virtual void			SetShader(EShaderType, ShaderBundle const&) = 0;
		virtual void			SetLayout(PipelineLayoutRef) = 0;
		virtual void			Finalize() = 0;
		virtual void			SetRasterizerState(const RasterizerState&) = 0;
		virtual void			SetBlendState(const BlendState&) = 0;
		virtual void			SetDepthStencilState(const DepthStencilState&) = 0;
		virtual void			SetPrimitiveTopology(const EPrimitiveType) = 0;
		virtual void			SetVertexInputLayout(rhi::VertexDeclaration const*, uint32 Count) = 0;
		virtual void			SetRenderTargetFormat(const RenderTargetFormat &) = 0;
		virtual void			SetSampler(SamplerRef) = 0;
		virtual void			SavePSO(const char* /*path*/) {}
		virtual void			LoadPSO(const char*) {}
	};

	struct IDescriptorPool
	{
		virtual ~IDescriptorPool() {}
	};

	//struct IPipelineBarrier
	//{
	//	virtual ~IPipelineBarrier() {}
	//};

	using PipelineLayoutDesc = shc::BindingTable;
	
	struct PipelineLayoutKey
	{
		uint32 BindingKey = 0;
		uint32 SetKey = 0;
		uint32 UniformKey = 0;
		bool operator==(PipelineLayoutKey const & rhs)
		{
			return BindingKey == rhs.BindingKey
				&& SetKey == rhs.SetKey 
				&& UniformKey == rhs.UniformKey;
		}
		bool operator<(PipelineLayoutKey const & rhs) const
		{
			return BindingKey < rhs.BindingKey
				|| SetKey < rhs.SetKey
				|| UniformKey < rhs.UniformKey;
		}
	};

	struct K3D_API IDevice
	{
		enum Result
		{
			DeviceNotFound,
			DeviceFound
		};

		virtual							~IDevice() {}
		virtual Result						Create(IDeviceAdapter *, bool withDebug) = 0;

		virtual CommandContextRef			NewCommandContext(ECommandType) = 0;
		virtual GpuResourceRef				NewGpuResource(ResourceDesc const&) = 0;
		virtual ShaderResourceViewRef		NewShaderResourceView(GpuResourceRef , ResourceViewDesc const&) = 0;
		virtual SamplerRef					NewSampler(const SamplerState&) = 0;
		virtual PipelineStateObjectRef		NewPipelineState(rhi::PipelineDesc const&,PipelineLayoutRef,EPipelineType) = 0;
		virtual PipelineLayoutRef			NewPipelineLayout(PipelineLayoutDesc const & table) = 0;
		virtual SyncFenceRef				NewFence() = 0;
		// deprecated interface
		virtual IDescriptorPool*			NewDescriptorPool() = 0; // should be abandoned
		virtual RenderViewportRef			NewRenderViewport(void * winHandle, GfxSetting &) = 0;
		virtual RenderTargetRef				NewRenderTarget(RenderTargetLayout const&) = 0;
		virtual void						WaitIdle() {}
		
		/* equal with d3d12's getcopyfootprint or vulkan's getImagesubreslayout.
		 */
		virtual void						QueryTextureSubResourceLayout(GpuResourceRef, TextureResourceSpec const& spec, SubResourceLayout *) {}
	};

	struct K3D_API IRenderViewport
	{
		virtual						~IRenderViewport() {}

		virtual bool				InitViewport(
										void *windowHandle, 
										IDevice * pDevice, 
										GfxSetting &
									) = 0;

		virtual void				PrepareNextFrame() {}


		/**
		 * @param	vSync	true to synchronise.
		 * @return	true if it succeeds, false if it fails.
		 */
		virtual bool				Present(bool vSync) = 0;

		virtual RenderTargetRef     GetRenderTarget(uint32 index) = 0;
		virtual RenderTargetRef		GetCurrentBackRenderTarget() = 0;

		virtual uint32				GetSwapChainCount() = 0;
		virtual uint32				GetSwapChainIndex() = 0;

		virtual uint32				GetWidth() const = 0;
		virtual uint32				GetHeight() const = 0;
	};

	struct IColorBuffer
	{
		virtual ~IColorBuffer() {}
	};

	struct IDepthBuffer 
	{
		virtual ~IDepthBuffer() {}
	};

	struct TextureCopyLocation
	{
		typedef ::k3d::DynArray<uint32> ResIds;
		typedef ::k3d::DynArray<PlacedSubResourceFootprint> ResFootprints;

		enum ESubResource
		{
			ESubResourceIndex,
			ESubResourceFootPrints
		};

		TextureCopyLocation(GpuResourceRef ptrResource, ResIds subResourceIndex)
		: pResource(ptrResource), SubResourceIndexes(subResourceIndex) {
		}

		TextureCopyLocation(GpuResourceRef ptrResource, ResFootprints footprints)
			: pResource(ptrResource), SubResourceFootPrints(footprints) {
		}
		
		TextureCopyLocation() = default;

		~TextureCopyLocation() {}

		GpuResourceRef	pResource;
		ESubResource	SubResourceType			= ESubResourceIndex;
		ResIds			SubResourceIndexes;
		ResFootprints	SubResourceFootPrints;
	};

	struct K3D_API ICommandContext
	{
		virtual ~ICommandContext() {}

		virtual void Detach(IDevice *) = 0;
		
		/**
		 * Like D3D12 Do
		 */
		virtual void CopyTexture(const TextureCopyLocation& Dest, const TextureCopyLocation& Src) = 0;
		
		virtual void CopyBuffer(IGpuResource& Dest, IGpuResource& Src, CopyBufferRegion const & Region) = 0;
		virtual void Execute(bool Wait) = 0;
		virtual void Reset() = 0;
		virtual void TransitionResourceBarrier(GpuResourceRef resource, /*EPipelineStage stage,*/ EResourceState dstState) = 0;

		virtual void Begin() {}
		virtual void End() {}
		virtual void PresentInViewport(RenderViewportRef) = 0;

		virtual void ClearColorBuffer(GpuResourceRef, kMath::Vec4f const&) = 0;
		virtual void ClearDepthBuffer(IDepthBuffer*) = 0;

		virtual void BeginRendering() = 0;
		virtual void SetRenderTargets(uint32 NumColorBuffer, IColorBuffer*, IDepthBuffer*, bool ReadOnlyDepth = false) = 0;
		virtual void SetRenderTarget(RenderTargetRef) = 0;
		virtual void SetScissorRects(uint32, const Rect*) = 0;
		virtual void SetViewport(const ViewportDesc &) = 0;
		virtual void SetIndexBuffer(const IndexBufferView& IBView) = 0;
		virtual void SetVertexBuffer(uint32 Slot, const VertexBufferView& VBView) = 0;
		virtual void SetPipelineState(uint32 HashCode, PipelineStateObjectRef) = 0;
		virtual void SetPipelineLayout(PipelineLayoutRef) = 0;
		virtual void SetPrimitiveType(EPrimitiveType) = 0;
		virtual void DrawInstanced(DrawInstancedParam) = 0;
		virtual void DrawIndexedInstanced(DrawIndexedInstancedParam) = 0;
		virtual void EndRendering() = 0;

		virtual void Dispatch(uint32 GroupCountX, uint32 GroupCountY, uint32 GroupCountZ) = 0;

		virtual void ExecuteBundle(ICommandContext*) {}
	};

}
