#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Private/VkUtils.h"
#include "Public/VkConfig.h"

K3D_VK_BEGIN

rhi::IDevice * DeviceAdapter::GetDevice()
{
	return new Device;
}

Device::Device() 
	: m_pGpu(nullptr)
	, m_Device(VK_NULL_HANDLE)
{
}

Device::~Device()
{
	Destroy();
}

void Device::Destroy()
{
	if (VK_NULL_HANDLE == m_Device)
		return;
	for (auto pll : m_CachedPipelineLayout)
	{
		pll.second->~PipelineLayout();
	}
	m_CachedPipelineLayout.clear();

	for (auto dsl : m_CachedDescriptorSetLayout)
	{
		dsl.second->~DescriptorSetLayout();
	}

	for (auto alloc : m_CachedDescriptorPool)
	{
		alloc.second->~DescriptorAllocator();
	}

	m_CachedDescriptorSetLayout.clear();
	m_PendingPass.~vector();
	m_ResourceManager->~ResourceManager();
	m_ContextPool->~CommandContextPool();

	vkDestroyDevice(m_Device, nullptr);
	VKLOG(Info, "Device-Destroyed");
	m_Device = VK_NULL_HANDLE;
}

rhi::IDevice::Result
Device::Create(rhi::IDeviceAdapter* pAdapter, bool withDbg)
{
	m_pGpu = static_cast<DeviceAdapter*>(pAdapter)->m_pGpu;
	VkPhysicalDevice& Gpu = *static_cast<DeviceAdapter*>(pAdapter)->m_pGpu;
	vkGetPhysicalDeviceMemoryProperties(Gpu, &m_MemoryProperties);
	VkResult err = CreateDevice(Gpu, withDbg, &m_Device);
	if (err)
	{
		VKLOG(Fatal, "Device-Create: Could not create Vulkan Device : %s.", ErrorString(err).c_str());
		return rhi::IDevice::DeviceNotFound;
	}
	else 
	{
		LoadVulkan(RHIRoot::GetInstance(), m_Device);
#if K3DPLATFORM_OS_WIN && _DEBUG
		if (withDbg)
		{
			SetupDebugging(RHIRoot::GetInstance(), VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, nullptr);
		}
#endif
		m_ResourceManager = std::make_unique<ResourceManager>(this, 1024, 1024);
		m_ContextPool = std::make_unique<CommandContextPool>(this);
		InitCmdQueue(VK_QUEUE_GRAPHICS_BIT, m_GfxQueueIndex, 0);
		return rhi::IDevice::DeviceFound;
	}
}

rhi::IRenderTarget * 
Device::NewRenderTarget(rhi::RenderTargetLayout const & layout)
{
	return new RenderTarget(this, layout);
}

void Device::InitCmdQueue(VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex)
{
	m_DefCmdQueue = std::make_shared<CommandQueue>(this, queueTypes, queueFamilyIndex, queueIndex);
}

VkResult Device::CreateDevice(VkPhysicalDevice gpu, bool withDebug, VkDevice * pDevice)
{
	uint32_t graphicsQueueIndex = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &m_QueueCount, NULL);
	K3D_ASSERT(m_QueueCount >= 1);
	VKLOG(Info, "Device-PhysicDeviceQueue count = %d.", m_QueueCount);
	std::vector<VkQueueFamilyProperties> queueProps;
	queueProps.resize(m_QueueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &m_QueueCount, queueProps.data());
	for (graphicsQueueIndex = 0; graphicsQueueIndex < m_QueueCount; graphicsQueueIndex++)
	{
		if (queueProps[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_GfxQueueIndex = graphicsQueueIndex;
			VKLOG(Info, "Device-graphicsQueueIndex(%d) queueFlags(%d).", graphicsQueueIndex, queueProps[graphicsQueueIndex].queueFlags);
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

	if (withDebug)
	{
		deviceCreateInfo.enabledLayerCount = (uint32)RHIRoot::s_LayerNames.size();
		deviceCreateInfo.ppEnabledLayerNames = RHIRoot::s_LayerNames.data();
	}

	if (enabledExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	return vkCreateDevice(gpu, &deviceCreateInfo, nullptr, pDevice);
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
	return m_ContextPool->RequestContext();
}

rhi::ISampler*			
Device::NewSampler(const rhi::SamplerState&)
{
	return nullptr;
}

rhi::IPipelineLayout *
Device::NewPipelineLayout(rhi::PipelineLayoutDesc const & table)
{
	// Hash the table parameter here, 
	// Lookup the layout by hash code
	rhi::PipelineLayoutKey key = HashPipelineLayoutDesc(table);
	if (m_CachedPipelineLayout.find(key) == m_CachedPipelineLayout.end())
	{
		m_CachedPipelineLayout[key] = new PipelineLayout(this, table);;
	}
	return m_CachedPipelineLayout[key];
}

DescriptorAllocator * 
Device::NewDescriptorAllocator(uint32 maxSets, BindingArray const & bindings)
{
	uint32 key = util::Hash32((const char*)bindings.Data(), bindings.Count() * sizeof(VkDescriptorSetLayoutBinding));
	if (m_CachedDescriptorPool.find(key) == m_CachedDescriptorPool.end())
	{
		DescriptorAllocator::Options options = {};
		m_CachedDescriptorPool[key] = new DescriptorAllocator(this, options, maxSets, bindings);
	}
	return m_CachedDescriptorPool[key];
}

DescriptorSetLayout * 
Device::NewDescriptorSetLayout(BindingArray const & bindings)
{
	uint32 key = util::Hash32((const char*)bindings.Data(), bindings.Count() * sizeof(VkDescriptorSetLayoutBinding));
	if (m_CachedDescriptorSetLayout.find(key) == m_CachedDescriptorSetLayout.end()) 
	{
		m_CachedDescriptorSetLayout[key] = new DescriptorSetLayout(this, bindings);
	}
	return m_CachedDescriptorSetLayout[key];
}

rhi::IPipelineStateObject*	
Device::NewPipelineState(rhi::PipelineDesc const & desc, rhi::IPipelineLayout * ppl, rhi::EPipelineType type)
{
	return CreatePipelineStateObject(desc, ppl);
}

rhi::IPipelineStateObject*	
Device::CreatePipelineStateObject(rhi::PipelineDesc const & desc, rhi::IPipelineLayout * ppl)
{
	return new PipelineStateObject(this, desc, static_cast<PipelineLayout*>(ppl));
}

rhi::ISyncFence * 
Device::NewFence()
{
	return new Fence(this);
}

rhi::IGpuResource*
Device::NewGpuResource(rhi::ResourceDesc const& Desc) 
{
	rhi::IGpuResource * resource = nullptr;
	switch (Desc.Type)
	{
	case rhi::EGT_Buffer:
		resource = new Buffer(this, Desc);
		break;
	case rhi::EGT_Texture1D:
		break;
	case rhi::EGT_Texture2D:
		resource = new Texture(this, Desc);
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
Device::NewRenderViewport(void * winHandle, rhi::GfxSetting& setting)
{
	return new RenderViewport(this, winHandle, setting);
}

PtrCmdAlloc Device::NewCommandAllocator(bool transient)
{
	return CommandAllocator::CreateAllocator(m_GfxQueueIndex, false, this);
}

PtrSemaphore Device::NewSemaphore()
{
	return std::make_shared<Semaphore>(this);
}

::k3d::IShaderCompiler *
Device::NewShaderCompiler()
{
	return CreateShaderCompiler(::k3d::EShaderLang::GLSL);
}

SwapChain * 
Device::NewSwapChain(rhi::GfxSetting const & setting)
{
	return nullptr;
}

K3D_VK_END