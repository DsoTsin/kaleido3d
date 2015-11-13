#pragma once
#ifndef __D3D12RHIResource_h__
#define __D3D12RHIResource_h__
#include "RHI/IRHI.h"
#include "RHI/D3D12/RHI.h"

NS_K3D_D3D12_BEGIN

class K3D_API GpuResource : public rhi::IGpuResource
{
public:
	GpuResource() :
		m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		m_UsageState(D3D12_RESOURCE_STATE_COMMON),
		m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{}

	GpuResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState) :
		m_Resource(pResource),
		m_UsageState(CurrentState),
		m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	virtual void Create(
		const kString& name,
		uint32_t NumElements, uint32_t ElementSize,
		const void* initialData = nullptr)
	{}

	//virtual D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const = 0;
	
	virtual ~GpuResource() {}

	void Destroy() {
		m_Resource = nullptr;
	}

	ID3D12Resource * GetResource() { return m_Resource.Get(); }

	void* Map();
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const;

	friend class CommandContext;
	friend class Device;

protected:

	Device*						m_Device;
	PtrResource					m_Resource;
	D3D12_RESOURCE_STATES		m_UsageState;
	D3D12_RESOURCE_STATES		m_TransitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS	m_GpuVirtualAddress;
};

class VertexBufferView : public rhi::VertexBufferView
{
	friend class CommandContext;
	friend class GpuBuffer;
public:
	VertexBufferView()
	{
	}

private:
	D3D12_VERTEX_BUFFER_VIEW m_BufferView;
};

class IndexBufferView : public rhi::IndexBufferView
{
	friend class CommandContext;
	friend class GpuBuffer;
public:
	IndexBufferView()
	{
	}

private:
	D3D12_INDEX_BUFFER_VIEW m_BufferView;
};

class ResourceViewHelper
{
public:
	virtual VertexBufferView AsVertexBufferView(uint64 Offset, uint32_t Size, uint32_t Stride) const = 0;
	virtual IndexBufferView AsIndexBufferView(uint64 Offset, uint32_t Size, bool b32Bit = false) const = 0;
};


///////////////////////////////////   Buffers  ////////////////////////////////////////////////////
class GpuBuffer : public GpuResource, public ResourceViewHelper
{
public:
	virtual ~GpuBuffer() { Destroy(); }

	virtual void Destroy(void);

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize,
		const void* initialData = nullptr) override;

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRV; }

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView(void) const { return m_GpuVirtualAddress; }

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t Offset, uint32_t Size) const;


	VertexBufferView AsVertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride)const override
	{
		VertexBufferView VBV;
		VBV.m_BufferView.BufferLocation = m_GpuVirtualAddress + Offset;
		VBV.m_BufferView.SizeInBytes = Size;
		VBV.m_BufferView.StrideInBytes = Stride;
		return VBV;
	}

	IndexBufferView AsIndexBufferView(size_t Offset, uint32_t Size, bool b32Bit = false)const override
	{
		IndexBufferView IBV;
		IBV.m_BufferView.BufferLocation = m_GpuVirtualAddress + Offset;
		IBV.m_BufferView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		IBV.m_BufferView.SizeInBytes = Size;
		return IBV;
	}

	VertexBufferView AsVertexBufferView(size_t BaseVertexIndex = 0) const
	{
		size_t Offset = BaseVertexIndex * m_ElementSize;
		return AsVertexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize);
	}

	IndexBufferView AsIndexBufferView(size_t StartIndex = 0) const
	{
		size_t Offset = StartIndex * m_ElementSize;
		return AsIndexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize == 4);
	}
protected:

	GpuBuffer(void) : m_BufferSize(0), m_ElementCount(0), m_ElementSize(0)
	{
		m_ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_UAV.ptr = ~0ull;
		m_SRV.ptr = ~0ull;
	}

	D3D12_RESOURCE_DESC DescribeBuffer(void);
	virtual void CreateDerivedViews(void) = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE m_UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SRV;

	size_t m_BufferSize;
	uint32_t m_ElementCount;
	uint32_t m_ElementSize;
	D3D12_RESOURCE_FLAGS m_ResourceFlags;
};


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


NS_K3D_D3D12_END

#endif