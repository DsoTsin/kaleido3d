#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Tools/ShaderGen/ShaderCompiler.h"
#include "Base/vulkantools.h"
#include "Private/VkUtils.h"

K3D_VK_BEGIN

rhi::IDevice * DeviceAdapter::GetDevice()
{
	return new Device;
}

Device::Device() 
	: m_pPhysicDevice(nullptr)
{
}

Device::~Device()
{
	m_ResourceManager->~ResourceManager();
	m_ContextPool->~CommandContextPool();
	vkDestroyDevice(m_Device, nullptr);
	Log::Out(LogLevel::Info, "Device", "Destroying..");
}

rhi::IDevice::Result
Device::Create(rhi::IDeviceAdapter* pAdapter, bool withDbg)
{
	m_pPhysicDevice = static_cast<DeviceAdapter*>(pAdapter)->m_pPDevice;
	VkPhysicalDevice& physicalDevice = *static_cast<DeviceAdapter*>(pAdapter)->m_pPDevice;
	uint32_t graphicsQueueIndex = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &m_QueueCount, NULL);
	K3D_ASSERT(m_QueueCount >= 1);
	Log::Out(LogLevel::Info, "Device", "PhysicDeviceQueue count = %d.", m_QueueCount);
	std::vector<VkQueueFamilyProperties> queueProps;
	queueProps.resize(m_QueueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &m_QueueCount, queueProps.data());
	for (graphicsQueueIndex = 0; graphicsQueueIndex < m_QueueCount; graphicsQueueIndex++)
	{
		if (queueProps[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_GfxQueueIndex = graphicsQueueIndex;
			Log::Out(LogLevel::Info, "Device", "graphicsQueueIndex(%d) queueFlags(%d).", graphicsQueueIndex, queueProps[graphicsQueueIndex].queueFlags);
			break;
		}
	}
	K3D_ASSERT(graphicsQueueIndex < m_QueueCount);

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
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_MemoryProperties);

	if (err)
	{
		Log::Out(LogLevel::Fatal, "Device", "Create: Could not create Vulkan Device : %s.", vkTools::errorString(err).c_str());
		return rhi::IDevice::DeviceNotFound;
	}
	else {
		m_ResourceManager.swap(std::make_unique<ResourceManager>(this, 1024, 1024));
		m_ContextPool.swap(std::make_unique<CommandContextPool>(this));
		vkGetDeviceQueue(m_Device, graphicsQueueIndex, 0, &m_DefaultQueue);
		return rhi::IDevice::DeviceFound;
	}
}


bool Device::FindMemoryType(uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex) const
{
#ifdef max
#undef max
	*typeIndex = std::numeric_limits<std::remove_pointer<decltype(typeIndex)>::type>::max();
#endif
	for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; ++i) {
		if (typeBits & 0x00000001) {
			if (requirementsMask == (m_MemoryProperties.memoryTypes[i].propertyFlags & requirementsMask)) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}

	return false;
}

rhi::ICommandContext*	
Device::NewCommandContext(rhi::ECommandType Type)
{
	rhi::ICommandContext * commandContext = nullptr;
	switch (Type)
	{
	case rhi::ECMD_Graphics:
		commandContext = m_ContextPool->RequestContext();
		break;
	case rhi::ECMD_Compute:
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
		pso = new PipelineStateObject(this);
		break;
	case rhi::EPSO_Compute:
		pso = new PipelineStateObject(this);
		break;
	}
	return pso;
}

rhi::ISyncFence * 
Device::NewFence()
{
	return new Fence(this);
}

rhi::IGpuResource*
Device::NewGpuResource(rhi::ResourceDesc const& Desc,uint64 Size) 
{
	rhi::IGpuResource * resource = nullptr;
	switch (Desc.Type)
	{
	case rhi::EGT_Buffer:
		resource = new Buffer(this, Size);
		break;
	case rhi::EGT_Texture1D:
		break;
	default:
		break;
	}
	return resource;
}

rhi::IDescriptorPool *
Device::NewDescriptorPool()
{
	return nullptr;
}

rhi::IRenderViewport * 
Device::NewRenderViewport(void * winHandle, uint32 width, uint32 height)
{
	return new RenderViewport(this, winHandle, width, height);
}

PtrCmdAlloc Device::NewCommandAllocator(bool transient)
{
	return CommandAllocator::CreateAllocator(m_GfxQueueIndex, false, this);
}

PtrSemaphore Device::NewSemaphore()
{
	return std::make_shared<Semaphore>(this);
}

k3d::IShaderCompiler * Device::NewShaderCompiler()
{
	return CreateShaderCompiler(::k3d::EShaderLang::GLSL);
}

K3D_VK_END