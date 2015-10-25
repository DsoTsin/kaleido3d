#pragma once
#ifndef __D3D12RHIResource_h__
#define __D3D12RHIResource_h__
#include "RHI/IRHI.h"
#include "RHI/D3D12/RHI.h"

NS_K3D_D3D12_BEGIN

class K3D_API GpuResource : public RHIRoot, public rhi::IGpuResource
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

	virtual ~GpuResource() {}

	void Destroy() {
		m_Resource = nullptr;
	}

	ID3D12Resource * GetResource() { return m_Resource.Get(); }

	void* Map();
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const;

	friend class GraphicsContext;
	friend class ComputeContext;
	friend class CommandContext;

protected:
	PtrResource m_Resource;
	D3D12_RESOURCE_STATES m_UsageState;
	D3D12_RESOURCE_STATES m_TransitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
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