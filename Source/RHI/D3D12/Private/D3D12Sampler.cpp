#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "../Utils.h"

NS_K3D_D3D12_BEGIN

namespace
{
	std::map< size_t, D3D12_CPU_DESCRIPTOR_HANDLE > s_SamplerCache;
}

void Sampler::Create(const D3D12_SAMPLER_DESC& Desc)
{
	size_t hashValue = HashState(&Desc);
	auto iter = s_SamplerCache.find(hashValue);
	if (iter != s_SamplerCache.end())
	{
		//*this = Sampler(iter->second);
		return;
	}
	/*
	m_hCpuDescriptorHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	g_Device->CreateSampler(&Desc, m_hCpuDescriptorHandle);
	*/
}

NS_K3D_D3D12_END