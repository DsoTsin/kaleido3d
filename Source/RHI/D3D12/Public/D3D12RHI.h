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
	rhi::IPipelineState*	NewPipelineState() override;
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
	friend class PipelineState;
public:
	VertexInputLayout(D3D12_INPUT_ELEMENT_DESC Descs[], uint32 Count)
		: m_InputLayout(Descs), m_ElementCount(Count) {}

private:
	std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]> m_InputLayout;
	uint32 m_ElementCount;
};

class PipelineState : public rhi::IPipelineState
{
	friend class Device;
public:

	PipelineState();

	void	SetShader(rhi::EShaderType, rhi::IShaderBytes*)override;
	void	SetRasterizerState(const rhi::RasterizerState&)override;
	void	SetBlendState(const rhi::BlendState&)override;
	void	SetDepthStencilState(const rhi::DepthStencilState&)override;
	void	SetSampler(rhi::ISampler*)override;
	void	SetVertexInputLayout(rhi::IVertexInputLayout *) override;
	void	SetPrimitiveTopology(const rhi::EPrimitiveType) override;


	void			Finalize();
	void			SetDevice(Device *);

	static void				DestroyAll(void);
	void					SetRootSignature(const RootSignature& BindMappings);
	const RootSignature&	GetRootSignature(void) const;
	ID3D12PipelineState*	GetPipelineStateObject(void) const { return m_pPSO.Get(); }

private:

	const RootSignature*							m_RootSignature;
	PtrPipeLineState								m_pPSO;
	union 
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC			m_GraphicsPSODesc;
		D3D12_COMPUTE_PIPELINE_STATE_DESC			m_ComputePSODesc;
	};
	shared_ptr<const D3D12_INPUT_ELEMENT_DESC>		m_InputLayouts;
	bool											m_IsComputeState;

	PtrDevice										m_Device;
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
	~CommandContext() override;

	void Initialize();

	void Detach(rhi::IDevice *) override;
	void CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src) override;
	void SetIndexBuffer(const rhi::IndexBufferView& IBView) override;
	void SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView) override;
	void SetPipelineState(uint32 hashCode, rhi::IPipelineState*)override;
	void SetViewport(const rhi::Viewport &)override;
	void SetPrimitiveType(rhi::EPrimitiveType)override;
	void DrawInstanced(rhi::DrawInstanceParam)override;
	void DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)override;
	void Execute(bool Wait) override;
	void Reset()override;

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	void SetDynamicVB(UINT Slot, size_t NumVertices, size_t VertexStride, const void* VBData);
	void SetDynamicIB(size_t IndexCount, const uint16_t* IBData);

	void FlushResourceBarriers();

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx);
	void ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries);

public:
	static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]) {}
	static void InitializeBuffer(GpuResource& Dest, const void* Data, size_t NumBytes) {}
	static void InitializeTextureArraySlice(GpuResource& Dest, UINT SliceIndex, GpuResource& Src) {}

private:

	DirectCommandListManager*	m_OwningManager;
	ID3D12GraphicsCommandList*	m_CommandList;
	ID3D12CommandAllocator*		m_CurrentAllocator;

	ID3D12RootSignature*		m_CurGraphicsRootSignature;
	ID3D12PipelineState*		m_CurGraphicsPipelineState;
	ID3D12RootSignature*		m_CurComputeRootSignature;
	ID3D12PipelineState*		m_CurComputePipelineState;

	DynamicDescriptorHeap		m_DynamicDescriptorHeap;

	D3D12_RESOURCE_BARRIER		m_ResourceBarrierBuffer[16];
	UINT						m_NumBarriersToFlush;

	ID3D12DescriptorHeap*		m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator				m_CpuLinearAllocator;
	LinearAllocator				m_GpuLinearAllocator;
};

extern DirectCommandListManager g_DirectCommandListManager;

NS_K3D_D3D12_END

#endif