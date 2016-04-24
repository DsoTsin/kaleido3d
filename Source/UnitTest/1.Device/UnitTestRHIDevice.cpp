#include "UnitTestRHIDevice.h"
#include <RHI/D3D12/DXCommon.h>
#include "RHI/D3D12/Public/D3D12RHI.h"
#include "RHI/Vulkan/VkCommon.h"
#include "RHI/Vulkan/Public/VkRHI.h"

K3D_APP_MAIN(UnitTestRHIDevice);

using namespace rhi;

bool UnitTestRHIDevice::OnInit()
{
	App::OnInit();
	IDeviceAdapter ** list = nullptr;
	uint32 adapterNum = 0;
	k3d::vk::EnumAllDeviceAdapter(list, &adapterNum);
	if (list == nullptr)
	{
		return false;
	}

	m_TestDevice = new k3d::vk::Device;
	IDevice::Result Result = m_TestDevice->Create(list[0], true);
	if (Result != IDevice::DeviceFound)
	{
		return false;
	}
	return true;
}

void UnitTestRHIDevice::OnDestroy()
{
	if (m_TestDevice)
	{
		delete m_TestDevice;
		m_TestDevice = nullptr;
	}
	App::OnDestroy();
}

void UnitTestRHIDevice::OnProcess(Message & msg)
{
}
