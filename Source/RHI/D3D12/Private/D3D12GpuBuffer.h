#ifndef __GpuBuffer_h__
#define __GpuBuffer_h__
#pragma once

#include "D3D12RHIPrivate.h"

NS_K3D_D3D12_BEGIN

class D3D12GpuBuffer : public D3D12RHIDeviceChild
{
	friend class Device;
	friend class CommandContext;
	friend class ComputeContext;
	friend class GraphicsContext;
public:
	D3D12GpuBuffer(Device::Ptr pDevice, uint64 SizeInBytes);

	virtual ~D3D12GpuBuffer();

private:
};

NS_K3D_D3D12_END

#endif