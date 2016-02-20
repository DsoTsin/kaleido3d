#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Base/vulkantools.h"

K3D_VK_BEGIN

Device::~Device()
{
	vkDestroyDevice(m_Device, nullptr);
	Log::Out(LogLevel::Info, "Device", "Destroying..");
}

rhi::IDevice::Result
Device::Create(rhi::IDeviceAdapter* pAdapter, bool withDbg)
{
	m_pPhysicDevice = static_cast<DeviceAdapter*>(pAdapter)->m_pPDevice;
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
		vkGetDeviceQueue(m_Device, graphicsQueueIndex, 0, &m_DefaultQueue);
		return rhi::IDevice::DeviceFound;
	}
}

rhi::ICommandContext*	
Device::NewCommandContext(rhi::ECommandType Type)
{
	rhi::ICommandContext * commandContext = nullptr;
	switch (Type)
	{
	case rhi::ECMD_Graphics:
		commandContext = new GraphicsCommandContext(this);
		break;
	case rhi::ECMD_Compute:
		commandContext = new ComputeCommandContext(this);
		break;
	}
	return commandContext;
}

rhi::ISampler*			
Device::NewSampler(const rhi::SamplerState&)
{
	return nullptr;
}

rhi::IPipelineStateObject*	
Device::NewPipelineState(rhi::EPipelineType type)
{
	rhi::IPipelineStateObject * pso = nullptr;
	switch (type)
	{
	case rhi::EPSO_Graphics:
		pso = new GraphicsPSO(this);
		break;
	case rhi::EPSO_Compute:
		pso = new ComputePSO(this);
		break;
	}
	return pso;
}

rhi::ISyncPointFence * 
Device::NewFence()
{
	return nullptr;
}

rhi::IGpuResource*
Device::NewGpuResource(rhi::EGpuResourceType type)
{
	rhi::IGpuResource * resource = nullptr;
	switch (type)
	{
	case rhi::EGT_Buffer:
		resource = new Buffer(this);
		break;
	case rhi::EGT_Texture1D:
		break;
	default:
		break;
	}
	return resource;
}
K3D_VK_END