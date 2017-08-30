#ifndef __GpuTexture_h__
#define __GpuTexture_h__
#pragma once

#include "D3D12RHIPrivate.h"
#include "D3D12Enums.h"

NS_K3D_D3D12_BEGIN

class D3D12GpuTexture : public D3D12RHIDeviceChild
{
public:

	D3D12GpuTexture(Device::Ptr pDevice, rhi::TextureDesc const & Desc);
	D3D12GpuTexture(Device::Ptr pDevice, PtrSwapChain SwapChain, uint32 BufferIndex);
	virtual ~D3D12GpuTexture();

	void CreateTextureView(rhi::EGpuMemViewType Type);

	DescriptorHandle GetHandle(rhi::EGpuMemViewType const & Type) const { return m_Handles[Type]; }

private:
	D3D12_RESOURCE_DESC	m_Desc;
	DescriptorHandle	m_Handles[rhi::GpuViewTypeNum];
	PtrResource			m_Resource;
};

NS_K3D_D3D12_END

#endif