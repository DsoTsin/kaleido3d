#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "D3D12GpuBuffer.h"
#include "D3D12GpuTexture.h"
#include <Math/kMath.hpp>

NS_K3D_D3D12_BEGIN

void * GpuResource::Map(uint64 start, uint64 size)
{
	void * bufferPointer;
	assert(m_Resource != nullptr);
	m_Range.Begin = start, m_Range.End = start+size;
	ThrowIfFailed(m_Resource->Map(0, &m_Range, reinterpret_cast<void**>(&bufferPointer)));
	return bufferPointer;
}

void GpuResource::UnMap() 
{
	m_Resource->Unmap(0, &m_Range);
}

D3D12_GPU_VIRTUAL_ADDRESS GpuResource::GetGpuVirtualAddress() const
{
	return m_Resource->GetGPUVirtualAddress();
}

rhi::GpuResourceRef
Device::NewGpuResource(rhi::ResourceDesc const& Desc)
{
	return rhi::GpuResourceRef();
}



///////////////////////////////////////////// Texture //////////////////////////////////////////////
#define TAG_D3D12GpuTexture "D3D12GpuTexture"

D3D12GpuTexture::D3D12GpuTexture(Device::Ptr pDevice, rhi::TextureDesc const & Desc)
	: D3D12RHIDeviceChild(pDevice)
{
	if (Desc.IsTex2D())
	{
		m_Desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	}
	else if (Desc.IsTex1D())
	{
		m_Desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	}
	else if (Desc.IsTex3D())
	{
		m_Desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	}
	m_Desc.Width = Desc.Width;
	m_Desc.Height = Desc.Height;
	m_Desc.DepthOrArraySize = static_cast<const UINT16>(Desc.Depth);
	m_Desc.MipLevels = static_cast<const UINT16>(Desc.MipLevels);
	m_Desc.Format = g_DXGIFormatTable[Desc.Format];

	m_Desc.SampleDesc.Count = 1;
	m_Desc.SampleDesc.Quality = 0;
	CD3DX12_HEAP_PROPERTIES HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(GetParentDeviceRef().Get()->CreateCommittedResource(&HeapProp, D3D12_HEAP_FLAG_NONE, &m_Desc, 
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_Resource.GetAddressOf())));
}

D3D12GpuTexture::D3D12GpuTexture(Device::Ptr pDevice, PtrSwapChain SwapChain, uint32 BufferIndex)
	: D3D12RHIDeviceChild(pDevice)
{
	SIZE_T Index;
	ThrowIfFailed(SwapChain->GetBuffer(BufferIndex , IID_PPV_ARGS(m_Resource.GetAddressOf())));
	DescriptorHeapAllocator & rtvAllocator = GetParentDeviceRef().GetViewDescriptorAllocator<D3D12_RENDER_TARGET_VIEW_DESC>();
	m_Handles[rhi::EGVT_RTV].CpuHandle = rtvAllocator.AllocateHeapSlot(Index);
}

D3D12GpuTexture::~D3D12GpuTexture()
{

}

void D3D12GpuTexture::CreateTextureView(rhi::EGpuMemViewType Type)
{
	SIZE_T Index;
	switch (Type)
	{
	case rhi::EGVT_RTV:
	{
		DescriptorHeapAllocator & rtvAllocator = GetParentDeviceRef().GetViewDescriptorAllocator<D3D12_RENDER_TARGET_VIEW_DESC>();
		m_Handles[rhi::EGVT_RTV].CpuHandle = rtvAllocator.AllocateHeapSlot(Index);
		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		GetParentDeviceRef().Get()->CreateRenderTargetView(m_Resource.Get(), &RTVDesc, m_Handles[rhi::EGVT_RTV].CpuHandle);
		break;
	}
	case rhi::EGVT_SRV: 
	{
		DescriptorHeapAllocator & srvAllocator = GetParentDeviceRef().GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>();
		m_Handles[rhi::EGVT_SRV].CpuHandle = srvAllocator.AllocateHeapSlot(Index);
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_Desc.Format;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // Texture2D
		//SRVDesc.Texture2D.MipLevels = NumMips;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		GetParentDeviceRef().Get()->CreateShaderResourceView(m_Resource.Get(), &SRVDesc, m_Handles[rhi::EGVT_SRV].CpuHandle);
		break;
	}
	case rhi::EGVT_UAV:
	{
		DescriptorHeapAllocator & uavAllocator = GetParentDeviceRef().GetViewDescriptorAllocator<D3D12_UNORDERED_ACCESS_VIEW_DESC>();
		m_Handles[rhi::EGVT_UAV].CpuHandle = uavAllocator.AllocateHeapSlot(Index);
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		break;
	}
	default:
		DX12LOG(Error, "D3D12GpuTexture::CreateTextureView unimplemented resource view type.");
		break;
	}
}

//////////////////////////////////////// Buffer /////////////////////////////////////////////////

D3D12GpuBuffer::D3D12GpuBuffer(Device::Ptr pDevice, uint64 SizeInBytes)
	: D3D12RHIDeviceChild(pDevice)
{

}

D3D12GpuBuffer::~D3D12GpuBuffer()
{

}

NS_K3D_D3D12_END
