#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Base/vulkantools.h"

K3D_VK_BEGIN

rhi::IDevice::Result
Device::Create(rhi::IDeviceAdapter* pAdapter, bool withDbg)
{
	VkPhysicalDevice& physicalDevice = *static_cast<DeviceAdapter*>(pAdapter)->m_pPDevice;
	uint32_t graphicsQueueIndex = 0;
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
	assert(queueCount >= 1);
	Log::Out(LogLevel::Info, "Device", "PhysicDeviceQueue count = %d.", queueCount);
	std::vector<VkQueueFamilyProperties> queueProps;
	queueProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

	for (graphicsQueueIndex = 0; graphicsQueueIndex < queueCount; graphicsQueueIndex++)
	{
		if (queueProps[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			Log::Out(LogLevel::Info, "Device", "graphicsQueueIndex(%d) queueFlags(%d).", graphicsQueueIndex, queueProps[graphicsQueueIndex].queueFlags);
			break;
		}
	}
	assert(graphicsQueueIndex < queueCount);

	std::array<float, 1> queuePriorities = { 0.0f };
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities.data();

	std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = NULL;

	if (enabledExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	VkResult err = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	if (err)
	{
		Log::Out(LogLevel::Fatal, "Device", "Create: Could not create Vulkan Device : %s.", vkTools::errorString(err).c_str());
		return rhi::IDevice::DeviceNotFound;
	}
	else {
		return rhi::IDevice::DeviceFound;
	}
}

rhi::ICommandContext*	
Device::NewCommandContext(rhi::ECommandType Type)
{
	return nullptr;
}

rhi::ISampler*			
Device::NewSampler(const rhi::SamplerState&)
{
	return nullptr;
}

rhi::IPipelineStateObject*	
Device::NewPipelineState(rhi::EPipelineType type)
{
	return nullptr;
}

rhi::ISyncPointFence * 
Device::NewFence()
{
	return nullptr;
}

rhi::IGpuResource*
Device::NewGpuResource(rhi::EGpuResourceType type)
{
	return nullptr;
}
K3D_VK_END