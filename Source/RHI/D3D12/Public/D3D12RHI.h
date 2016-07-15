#pragma once
#ifndef __D3D12RHI_h__
#define __D3D12RHI_h__

#include <Math/kMath.hpp>
#include "RHI/IRHI.h"
#include "Private/D3D12RootSignature.h"
#include "Private/D3D12CommandListManager.h"
#include "Public/D3D12RenderTarget.h"

NS_K3D_D3D12_BEGIN

extern "C++" K3D_API void EnumAllDeviceAdapter(rhi::IDeviceAdapter** &, uint32*);

class DeviceAdapter : public rhi::IDeviceAdapter
{
public:
	rhi::IDevice * GetDevice() override;

	enum Vendor {
		AMD,
		Intel,
		NVIDIA,
		UnKnown
	};
	DeviceAdapter() : m_Vendor(Vendor::UnKnown) {};
	explicit DeviceAdapter(PtrAdapter adapter, Vendor vendor = Vendor::UnKnown) 
		: m_Adapter(adapter), m_Vendor(vendor) {}

	friend class Device;
private:
	PtrAdapter	m_Adapter;
	Vendor		m_Vendor;
};

typedef DeviceAdapter::Vendor GpuVendor;

class CommandContext;

class DescriptorHeapAllocator
{
public:
	DescriptorHeapAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap);

	void							Init(PtrDevice pDevice);
	D3D12_CPU_DESCRIPTOR_HANDLE		AllocateHeapSlot(SIZE_T &outIndex);
	void							FreeHeapSlot(D3D12_CPU_DESCRIPTOR_HANDLE Offset, SIZE_T index);

private:
	struct Node
	{
		SIZE_T Start;
		SIZE_T End;
		bool operator==(Node const & rhs) {
			return rhs.Start == Start && rhs.End == End;
		}
	};

	typedef std::list<Node> NodeList;

	struct Entry
	{
		PtrDescHeap m_Heap;
		NodeList	m_FreeList;
		Entry() {}
		Entry(PtrDescHeap heap, NodeList list) :m_Heap(heap), m_FreeList(list) {}
	};

	typedef std::vector<Entry> THeapMap;

private:
	/*!
	\brief	Allocate heap only once.
	*/
	void AllocateHeap();

private:
	const D3D12_DESCRIPTOR_HEAP_DESC	m_Desc;
	uint32								m_DescriptorSize;
	PtrDevice							m_Device;

	std::vector<Entry>					m_Heaps;
	std::list<SIZE_T>					m_FreeHeaps;

	std::mutex							m_AllocMutex;
};

class Device : public rhi::IDevice, public std::enable_shared_from_this<Device>
{
public:
	typedef Device*			Ptr;

	explicit				Device(bool useWarp = false);
							~Device();

	rhi::IDevice::Result	Create(
		rhi::IDeviceAdapter* adapter,
		bool withDbg = true) override;

	rhi::ICommandContext*	NewCommandContext(rhi::ECommandType)override;
	rhi::IGpuResource*		NewGpuResource(rhi::ResourceDesc const&) override;
	rhi::ISampler*			NewSampler(const rhi::SamplerState&) override;

	rhi::IPipelineLayout*		NewPipelineLayout(rhi::PipelineLayoutDesc const & table) override;
	rhi::IPipelineStateObject*	NewPipelineState(rhi::PipelineDesc const & desc, rhi::IPipelineLayout * ppl, rhi::EPipelineType) override;
	rhi::ISyncFence*		NewFence() override;
	rhi::IDescriptorPool *	NewDescriptorPool() override;
	rhi::IRenderViewport *	NewRenderViewport(void * winHandle, rhi::GfxSetting&) override;
	::k3d::IShaderCompiler *NewShaderCompiler() override;

	rhi::IRenderTarget*		NewRenderTarget(rhi::RenderTargetLayout const & layout) override;

	PtrDevice				Get()
	{
		return m_Inst;
	}

	PtrGIFactory			GetDXGIFactory()
	{
		return m_Factory;
	}

	CommandContext &		GetDefaultContext() const
	{
		K3D_ASSERT(m_DefaultContext);
		return *m_DefaultContext;
	}

	template <typename TViewDesc> DescriptorHeapAllocator& GetViewDescriptorAllocator();
	template<> DescriptorHeapAllocator& GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>() { return m_SRVDHAllocator; }
	template<> DescriptorHeapAllocator& GetViewDescriptorAllocator<D3D12_RENDER_TARGET_VIEW_DESC>() { return m_RTVDHAllocator; }
	template<> DescriptorHeapAllocator& GetViewDescriptorAllocator<D3D12_UNORDERED_ACCESS_VIEW_DESC>() { return m_UAVDHAllocator; }
	template<> DescriptorHeapAllocator& GetViewDescriptorAllocator<D3D12_CONSTANT_BUFFER_VIEW_DESC>() { return m_CBVDHAllocator; }
	template<> DescriptorHeapAllocator& GetViewDescriptorAllocator<D3D12_DEPTH_STENCIL_VIEW_DESC>() { return m_DSVDHAllocator; }
	template<> DescriptorHeapAllocator& GetViewDescriptorAllocator<D3D12_SAMPLER_DESC>() { return m_SamplerDHAllocator; }

private:
	bool				m_Warp;
	PtrDevice			m_Inst;
	PtrGIFactory		m_Factory;

protected:
	shared_ptr<CommandContext> m_DefaultContext;
	DescriptorHeapAllocator m_RTVDHAllocator;
	DescriptorHeapAllocator m_UAVDHAllocator;
	DescriptorHeapAllocator m_SRVDHAllocator;
	DescriptorHeapAllocator m_CBVDHAllocator;
	DescriptorHeapAllocator m_DSVDHAllocator;
	DescriptorHeapAllocator m_SamplerDHAllocator;
};

class D3D12RHIDeviceChild
{
public:
	D3D12RHIDeviceChild(Device::Ptr pDevice) 
		: m_Device(pDevice) 
	{
	}

	Device& GetParentDeviceRef() const
	{
		return *m_Device;
	}

	Device::Ptr GetParentDevicePtr() const
	{
		return m_Device;
	}

	PtrDevice GetRawDeviceRef() const { return m_Device->Get(); }

	template <class T>
	inline DescriptorHeapAllocator& GetViewDescriptorAllocator() 
	{
		return m_Device->template GetViewDescriptorAllocator<T>();
	}

private:
	Device::Ptr m_Device;
};

struct ShaderBytes : public rhi::IShaderBytes
{
	ShaderBytes(PtrBlob Blob) : ShaderBC(Blob) {}

	uint32	Length() override
	{
		return static_cast<uint32>(ShaderBC->GetBufferSize());
	}
	const void*	Bytes() override
	{
		return ShaderBC->GetBufferPointer();
	}
	D3D12_SHADER_BYTECODE AsBC()
	{
		return D3D12_SHADER_BYTECODE {ShaderBC->GetBufferPointer(), ShaderBC->GetBufferSize() };
	}

	PtrBlob ShaderBC;
};

/*
struct ShaderCompiler : public rhi::IShaderCompiler
{
	rhi::IShaderBytes* CompileFromSource(ELangVersion, rhi::EShaderType, const char*) override;
	rhi::IShaderBytes* CompileFromSource(ELangVersion, rhi::EShaderType, const char*, const char * entry);
};
*/
class RootSignature;

class PipelineLayout : public rhi::IPipelineLayout
{
	friend class PipelineState;
public:
	PipelineLayout();
	explicit PipelineLayout(rhi::ShaderParamLayout const &);
	~PipelineLayout();

	void Create(rhi::ShaderParamLayout const &);
	// bad implemented
	void Finalize(rhi::IDevice *);

	ID3D12RootSignature * GetRootSignature() const { return m_RootSignature.GetSignature(); }
private:

	void CreateFromShaderLayout(rhi::ShaderParamLayout const &);

	RootSignature m_RootSignature;
};

class PipelineState : public rhi::IPipelineStateObject, public D3D12RHIDeviceChild
{
	friend class Device;
public:

	PipelineState(Device::Ptr pDevice);
	virtual ~PipelineState();

	void	SetShader(rhi::EShaderType, ::k3d::IShaderCompilerOutput*)override;
	void	SetLayout(rhi::IPipelineLayout *) override;
	void	Finalize() override;
	virtual rhi::EPipelineType	GetType() { return rhi::EPSO_Graphics; }
	void	SetRasterizerState(const rhi::RasterizerState&)override;
	void	SetBlendState(const rhi::BlendState&)override;
	void	SetDepthStencilState(const rhi::DepthStencilState&)override;
	void	SetSampler(rhi::ISampler*)override;
	void	SetVertexInputLayout(rhi::VertexDeclaration const*, uint32 Count) override;
	void	SetPrimitiveTopology(const rhi::EPrimitiveType) override;
	void	SetRenderTargetFormat(const rhi::RenderTargetFormat &) override;

	void	SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0)
	{
		SetRenderTargetFormats(1, &RTVFormat, DSVFormat, MsaaCount, MsaaQuality);
	}

	void	SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality);
		static void				DestroyAll(void);
	void					SetRootSignature(const RootSignature& BindMappings);
	const RootSignature&	GetRootSignature(void) const;
	ID3D12PipelineState*	GetPipelineStateObject(void) const { return m_pPSO.Get(); }

protected:
	union {
		D3D12_COMPUTE_PIPELINE_STATE_DESC			m_ComputePSODesc;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC			m_GraphicsPSODesc;
	};
	const RootSignature*							m_RootSignature;
	PtrPipeLineState								m_pPSO;
	shared_ptr<const D3D12_INPUT_ELEMENT_DESC>		m_InputLayouts;
};

class SyncPointFence : public rhi::ISyncFence, public D3D12RHIDeviceChild
{
	friend class Device;
public:
	SyncPointFence(Device::Ptr pDevice) : D3D12RHIDeviceChild(pDevice)
	{
		GetRawDeviceRef()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
	}
	~SyncPointFence() override {}

	void Signal(int32 fenceVal) override { m_Fence->Signal(fenceVal); }
	void Reset() override { }
	void WaitFor(uint64 time) override { }
private:
	PtrFence m_Fence;
};

class GpuResource : public rhi::IGpuResource, public D3D12RHIDeviceChild
{
public:
	explicit GpuResource(Device::Ptr pDevice) 
		: D3D12RHIDeviceChild(pDevice)
		, m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		, m_UsageState(D3D12_RESOURCE_STATE_COMMON)
		, m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{}

	GpuResource(Device::Ptr pDevice, ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState)
		: D3D12RHIDeviceChild(pDevice)
		, m_Resource(pResource)
		, m_UsageState(CurrentState)
		, m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	virtual void Create(
		const kString& name,
		uint32_t NumElements, uint32_t ElementSize,
		const void* initialData = nullptr)
	{}

	virtual ~GpuResource()
	{
	}

	void						Destroy() { m_Resource = nullptr; }
	ID3D12Resource *			GetResource() { return m_Resource.Get(); }
	void*						Map(uint64 start, uint64 size) override;
	void						UnMap() override;
	D3D12_GPU_VIRTUAL_ADDRESS	GetGpuVirtualAddress() const;
	uint64						GetResourceSize() const override { return -1; }
	friend class CommandContext;

protected:
	D3D12_RANGE					m_Range;
	PtrResource					m_Resource;
	D3D12_RESOURCE_STATES		m_UsageState;
	D3D12_RESOURCE_STATES		m_TransitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS	m_GpuVirtualAddress;
};


///////////////////////////////// Gpu Resource Allocation Manager ////////////////////////////////////////////

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256

// Various types of allocations may contain NULL pointers.  Check before dereferencing if you are unsure.
struct DynAlloc
{
	DynAlloc(GpuResource& BaseResource, size_t ThisOffset, size_t ThisSize)
		: Buffer(BaseResource), Offset(ThisOffset), Size(ThisSize) {}

	GpuResource& Buffer;	// The D3D buffer associated with this memory.
	size_t Offset;			// Offset from start of buffer resource
	size_t Size;			// Reserved size of this allocation
	void* DataPtr;			// The CPU-writeable address
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;	// The GPU-visible address
};

class LinearAllocationPage : public GpuResource
{
public:
	LinearAllocationPage(Device::Ptr pDevice, ID3D12Resource* pResource, D3D12_RESOURCE_STATES Usage) 
		: GpuResource(pDevice)
	{
		m_Resource.Attach(pResource);
		m_UsageState = Usage;
		m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();
		m_Resource->Map(0, nullptr, &m_CpuVirtualAddress);
	}

	~LinearAllocationPage()
	{
		m_Resource->Unmap(0, nullptr);
	}

	void* m_CpuVirtualAddress;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};

enum LinearAllocatorType
{
	kInvalidAllocator = -1,

	kGpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
	kCpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

	kNumAllocatorTypes
};

enum
{
	kGpuAllocatorPageSize = 0x10000,	// 64K
	kCpuAllocatorPageSize = 0x200000	// 2MB
};

class LinearAllocatorPageManager : public D3D12RHIDeviceChild
{
public:

	explicit LinearAllocatorPageManager(Device::Ptr pDevice);
	LinearAllocationPage* RequestPage(void);
	void DiscardPages(uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages);

	void Destroy(void) { m_PagePool.clear(); }

private:
	K3D_DISCOPY(LinearAllocatorPageManager)

	LinearAllocationPage* CreateNewPage(void);

	static LinearAllocatorType sm_AutoType;

	LinearAllocatorType m_AllocationType;
	std::vector<std::unique_ptr<LinearAllocationPage> > m_PagePool;
	std::queue<std::pair<uint64_t, LinearAllocationPage*> > m_RetiredPages;
	std::queue<LinearAllocationPage*> m_AvailablePages;
	std::mutex m_Mutex;
};

class LinearAllocator
{
public:

	LinearAllocator(LinearAllocatorType Type, Device::Ptr pDevice) : m_AllocationType(Type), m_PageSize(0), m_CurOffset(~0ull), m_CurPage(nullptr)
	{
		K3D_ASSERT(Type > kInvalidAllocator && Type < kNumAllocatorTypes);
		m_PageSize = (Type == kGpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
		sm_PageManager[0] = new LinearAllocatorPageManager(pDevice);
		sm_PageManager[1] = new LinearAllocatorPageManager(pDevice);
	}

	DynAlloc Allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

	void CleanupUsedPages(uint64_t FenceID);

	void DestroyAll(void)
	{
		sm_PageManager[0]->Destroy();
		sm_PageManager[1]->Destroy();
	}

private:

	K3D_DISCOPY(LinearAllocator)

	LinearAllocatorPageManager * sm_PageManager[2];

	LinearAllocatorType m_AllocationType;
	size_t m_PageSize;
	size_t m_CurOffset;
	LinearAllocationPage* m_CurPage;
	std::vector<LinearAllocationPage*> m_RetiredPages;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class Sampler : public rhi::ISampler
{
	friend class CommandContext;
public:
	typedef ISampler Type;
	Sampler() {}
	Sampler(D3D12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor)
		: m_hCpuDescriptorHandle(hCpuDescriptor) {}

	void Create(const D3D12_SAMPLER_DESC& Desc);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle() const { return m_hCpuDescriptorHandle; }

protected:

	D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuDescriptorHandle;
};

class CommandContext : public rhi::ICommandContext
{
	friend class Device;
public:
	typedef std::shared_ptr<CommandContext> Ptr;

	explicit CommandContext(Device::Ptr pDevice);
	virtual ~CommandContext() override;

	void Initialize();

	void Detach(rhi::IDevice *) override;
	void CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src, ::k3d::DynArray<rhi::BufferRegion> const& Regions) override;
	void Execute(bool Wait) override;
	void Reset()override;

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);

	void FlushResourceBarriers();

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginRendering() override {}
	void EndRendering() override {}
	void InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx);
	void ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries);

	void ClearColorBuffer(rhi::IGpuResource*, kMath::Vec4f const & color)override;
	void ClearDepthBuffer(rhi::IDepthBuffer* iDepthBuffer)override;
	void SetRenderTarget(rhi::IRenderTarget * rt) override;

	void SetRenderTargets(
		uint32 NumColorBuffer, rhi::IColorBuffer * ColorBuffers,
		rhi::IDepthBuffer * iDepthBuffer, bool ReadOnlyDepth = false) override;
	/*
	void SetRenderTargets(
	uint32 NumRTs, D3D12RenderTarget * pRTs);
	*/
	void SetViewport(const rhi::ViewportDesc &)override;
	void SetScissorRects(uint32, const rhi::Rect*)override;

	void SetIndexBuffer(const rhi::IndexBufferView& IBView) override;
	void SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView) override;
	void SetPipelineState(uint32 hashCode, rhi::IPipelineStateObject*)override;
	void SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout) override;
	void SetPrimitiveType(rhi::EPrimitiveType)override;
	void DrawInstanced(rhi::DrawInstanceParam)override;
	void DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)override;
	void Dispatch(uint32 X, uint32 Y, uint32 Z) override;
	void TransitionResourceBarrier(rhi::IGpuResource * resource,
		rhi::EResourceState srcState, rhi::EResourceState dstState) override;
	void SetRootSignature(const RootSignature &);

	void PresentInViewport(rhi::IRenderViewport *) override {}

	DirectCommandListManager & GetCommandListManager() const
	{
		K3D_ASSERT(m_OwningManager != nullptr);
		return *m_OwningManager;
	}
public:
	static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]) {}
	static void InitializeBuffer(GpuResource& Dest, const void* Data, size_t NumBytes) {}
	static void InitializeTextureArraySlice(GpuResource& Dest, UINT SliceIndex, GpuResource& Src) {}

protected:

	DirectCommandListManager*	m_OwningManager;
	ID3D12GraphicsCommandList*	m_CommandList;
	ID3D12CommandAllocator*		m_CurrentAllocator;

	//DynamicDescriptorHeap		m_DynamicDescriptorHeap;

	D3D12_RESOURCE_BARRIER		m_ResourceBarrierBuffer[16];
	UINT						m_NumBarriersToFlush;

	ID3D12DescriptorHeap*		m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator				m_CpuLinearAllocator;
	LinearAllocator				m_GpuLinearAllocator;

	ID3D12RootSignature*		m_CurGraphicsRootSignature;
	ID3D12PipelineState*		m_CurGraphicsPipelineState;

	ID3D12RootSignature*		m_CurComputeRootSignature;
	ID3D12PipelineState*		m_CurComputePipelineState;
};

extern DirectCommandListManager g_DirectCommandListManager;
extern const char * TAG_COMMAND_CONTEXT;
extern const char * TAG_COMPUTE_CONTEXT;
extern const char * TAG_GRAPHICS_CONTEXT;

NS_K3D_D3D12_END

#endif