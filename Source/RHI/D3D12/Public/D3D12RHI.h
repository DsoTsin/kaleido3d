#pragma once
#ifndef __D3D12RHI_h__
#define __D3D12RHI_h__

#include "D3D12RHIResource.h"
#include "RHI/D3D12/Private/LinearAllocator.h"
#include "RHI/D3D12/Private/D3D12CommandListManager.h"
#include "../DescriptorHeap.h"

NS_K3D_D3D12_BEGIN

class DeviceAdapter : public rhi::IDeviceAdapter
{
public:
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

class Device : public rhi::IDevice
{
public:

	explicit				Device(bool useWarp = false);
							~Device();

	rhi::IDevice::Result	Create(
		rhi::IDeviceAdapter* adapter,
		bool withDbg = true) override;

	rhi::ICommandContext*	NewCommandContext(rhi::ECommandType)override;
	rhi::IGpuResource*		NewGpuResource(rhi::EGpuResourceType type) override;
	rhi::ISampler*			NewSampler(const rhi::SamplerState&) override;
	rhi::IPipelineStateObject*	NewPipelineState(rhi::EPipelineType) override;
	rhi::ISyncPointFence*	NewFence() override;

	PtrDevice				Get()
	{
		return m_Inst;
	}

	PtrGIFactory			GetDXGIFactory()
	{
		return m_Factory;
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

struct ShaderCompiler : public rhi::IShaderCompiler
{
	rhi::IShaderBytes* CompileFromSource(ELangVersion, rhi::EShaderType, const char*) override;
	rhi::IShaderBytes* CompileFromSource(ELangVersion, rhi::EShaderType, const char*, const char * entry);
};

class RootSignature;

class VertexInputLayout : public rhi::IVertexInputLayout
{
	friend class GraphicsPSO;
public:
	VertexInputLayout(D3D12_INPUT_ELEMENT_DESC Descs[], uint32 Count)
		: m_InputLayout(Descs), m_ElementCount(Count) {}

private:
	std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]> m_InputLayout;
	uint32 m_ElementCount;
};

class PipelineLayout : public rhi::IPipelineLayout
{
	friend class PipelineState;
public:
	PipelineLayout();
	explicit PipelineLayout(rhi::ShaderParamLayout const &);
	~PipelineLayout();

	void Create(rhi::ShaderParamLayout const &) override;
	// bad implemented
	void Finalize(rhi::IDevice *)override;

	ID3D12RootSignature * GetRootSignature() const { return m_RootSignature.GetSignature(); }
private:

	void CreateFromShaderLayout(rhi::ShaderParamLayout const &);

	RootSignature m_RootSignature;
};

class PipelineState : public rhi::IPipelineStateObject
{
	friend class Device;
public:

	PipelineState();

	void	SetShader(rhi::EShaderType, rhi::IShaderBytes*)override;
	void	SetLayout(rhi::IPipelineLayout *) override;
	void	Finalize() override;
	virtual rhi::EPipelineType	GetType() = 0;

	static void				DestroyAll(void);
	void					SetDevice(Device *);
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
	PtrDevice										m_Device;
};

class GraphicsPSO : public PipelineState, public rhi::IGraphicsPipelineState
{
public:
	GraphicsPSO();
	~GraphicsPSO();

	void	SetRasterizerState(const rhi::RasterizerState&)override;
	void	SetBlendState(const rhi::BlendState&)override;
	void	SetDepthStencilState(const rhi::DepthStencilState&)override;
	void	SetSampler(rhi::ISampler*)override;
	void	SetVertexInputLayout(rhi::IVertexInputLayout *) override;
	void	SetPrimitiveTopology(const rhi::EPrimitiveType) override;
	void	SetRenderTargetFormat(const rhi::RenderTargetFormat &) override;

	void	SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0)
	{
		SetRenderTargetFormats(1, &RTVFormat, DSVFormat, MsaaCount, MsaaQuality);
	}

	void	SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality);

	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Graphics;
	}

private:
	shared_ptr<const D3D12_INPUT_ELEMENT_DESC>		m_InputLayouts;
};


class ComputePSO :public PipelineState, public rhi::IComputePipelineState
{
public:
	ComputePSO();
	~ComputePSO();

	rhi::EPipelineType GetType() override
	{
		return rhi::EPipelineType::EPSO_Compute;
	}

private:

	D3D12_COMPUTE_PIPELINE_STATE_DESC			m_ComputePSODesc;
};

class SyncPointFence : public rhi::ISyncPointFence
{
	friend class Device;
public:
	SyncPointFence();
	~SyncPointFence();

private:
	PtrFence m_Fence;
};

class CommandContext : public rhi::ICommandContext
{
	friend class Device;
public:
	CommandContext();
	virtual ~CommandContext() override;

	void Initialize();

	void Detach(rhi::IDevice *) override;
	void CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src) override;
	void Execute(bool Wait) override;
	void Reset()override;
	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);

	void FlushResourceBarriers();

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx);
	void ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries);

public:
	static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]) {}
	static void InitializeBuffer(GpuResource& Dest, const void* Data, size_t NumBytes) {}
	static void InitializeTextureArraySlice(GpuResource& Dest, UINT SliceIndex, GpuResource& Src) {}

protected:

	DirectCommandListManager*	m_OwningManager;
	ID3D12GraphicsCommandList*	m_CommandList;
	ID3D12CommandAllocator*		m_CurrentAllocator;

	DynamicDescriptorHeap		m_DynamicDescriptorHeap;

	D3D12_RESOURCE_BARRIER		m_ResourceBarrierBuffer[16];
	UINT						m_NumBarriersToFlush;

	ID3D12DescriptorHeap*		m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator				m_CpuLinearAllocator;
	LinearAllocator				m_GpuLinearAllocator;
};

struct ID3D12ColorBuffer : public rhi::IColorBuffer
{
	virtual void CreateFromSwapChain(const kString& Name, ID3D12Resource* BaseResource) = 0;
};


class GraphicsContext : 
	virtual public rhi::IGraphicsCommand,
	virtual public CommandContext
{
public:

	GraphicsContext();
	~GraphicsContext() override;

	void ClearColorBuffer(rhi::IColorBuffer* iColorBuffer)override;

	void ClearDepthBuffer(rhi::IDepthBuffer* iDepthBuffer)override;

	void SetRenderTargets(
		uint32 NumColorBuffer, rhi::IColorBuffer * ColorBuffers, 
		rhi::IDepthBuffer * iDepthBuffer, bool ReadOnlyDepth = false) override;

	void SetViewport(const rhi::Viewport &)override;
	void SetScissorRects(uint32, const rhi::Rect*)override;

	void SetIndexBuffer(const rhi::IndexBufferView& IBView) override;
	void SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView) override;
	void SetPipelineState(uint32 hashCode, rhi::IPipelineStateObject*)override;
	void SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout) override;
	void SetPrimitiveType(rhi::EPrimitiveType)override;
	void DrawInstanced(rhi::DrawInstanceParam)override;
	void DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)override;

	void SetRootSignature(const RootSignature &);

private:

	ID3D12RootSignature*		m_CurGraphicsRootSignature;
	ID3D12PipelineState*		m_CurGraphicsPipelineState;
};


class ComputeContext :
	virtual public rhi::IComputeCommand,
	virtual public CommandContext
{
public:
	ComputeContext();
	~ComputeContext() override;

	void SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout) override;
	void Dispatch(uint32 X, uint32 Y, uint32 Z) override;

private:

	ID3D12RootSignature*		m_CurComputeRootSignature;
	ID3D12PipelineState*		m_CurComputePipelineState;
};

extern DirectCommandListManager g_DirectCommandListManager;
extern const char * TAG_COMMAND_CONTEXT;
extern const char * TAG_COMPUTE_CONTEXT;
extern const char * TAG_GRAPHICS_CONTEXT;

NS_K3D_D3D12_END

#endif