#include "VkCommon.h"
#include "VkRHI.h"
#include "VkUtils.h"
#include "VkConfig.h"

using namespace rhi;

K3D_VK_BEGIN

DeviceRef DeviceAdapter::GetDevice()
{
	if (!m_pDevice)
	{
		m_pDevice = MakeShared<Device>();
	}
	return m_pDevice;
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

IDevice::Result
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
		m_DefCmdQueue = InitCmdQueue(VK_QUEUE_GRAPHICS_BIT, m_GraphicsQueueIndex, 0);
		m_ComputeCmdQueue = InitCmdQueue(VK_QUEUE_COMPUTE_BIT, m_ComputeQueueIndex, 0);
		return rhi::IDevice::DeviceFound;
	}
}

RenderTargetRef
Device::NewRenderTarget(rhi::RenderTargetLayout const & layout)
{
	return RenderTargetRef(new RenderTarget(this, layout));
}

uint64 Device::GetMaxAllocationCount()
{
	return m_PhysicalDeviceProperties.limits.maxMemoryAllocationCount;
}

SpCmdQueue Device::InitCmdQueue(VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex)
{
	return std::make_shared<CommandQueue>(this, queueTypes, queueFamilyIndex, queueIndex);
}

VkResult Device::CreateDevice(VkPhysicalDevice gpu, bool withDebug, VkDevice * pDevice)
{
	// get all device queues and find graphics queue
	GetDeviceQueueProps(gpu);

	// get device limits
	vkGetPhysicalDeviceProperties(gpu, &m_PhysicalDeviceProperties);

	std::array<float, 1> queuePriorities = { 0.0f };
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = m_GraphicsQueueIndex;
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

bool Device::GetDeviceQueueProps(VkPhysicalDevice gpu)
{
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &m_QueueCount, NULL);
	if(m_QueueCount < 1)
		return false;
	VKLOG(Info, "Device-PhysicDeviceQueue count = %d.", m_QueueCount);
	queueProps.resize(m_QueueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &m_QueueCount, queueProps.data());
	uint32 qId = 0;
	for (qId = 0; qId < m_QueueCount; qId++)
	{
		if (queueProps[qId].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_GraphicsQueueIndex = qId;
			VKLOG(Info, "Device-graphicsQueueIndex(%d) queueFlags(%d).", m_GraphicsQueueIndex, queueProps[qId].queueFlags);
			break;
		}
	}
	for (qId = 0; qId < m_QueueCount; qId++)
	{
		if (queueProps[qId].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			m_ComputeQueueIndex = qId;
			VKLOG(Info, "Device::ComputeQueueIndex(%d).", m_ComputeQueueIndex);
			break;
		}
	}
	for (qId = 0; qId < m_QueueCount; qId++)
	{
		if (queueProps[qId].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			m_CopyQueueIndex = qId;
			VKLOG(Info, "Device::CopyQueueIndex(%d).", m_CopyQueueIndex);
			break;
		}
	}
	return qId < m_QueueCount;
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

CommandContextRef
Device::NewCommandContext(rhi::ECommandType Type)
{
	return CommandContextRef(m_ContextPool->RequestContext(Type));
}

SamplerRef
Device::NewSampler(const rhi::SamplerState& samplerDesc)
{
	return MakeShared<Sampler>(this, samplerDesc);
}

PipelineLayoutRef
Device::NewPipelineLayout(rhi::PipelineLayoutDesc const & table)
{
	// Hash the table parameter here, 
	// Lookup the layout by hash code
	rhi::PipelineLayoutKey key = HashPipelineLayoutDesc(table);
	if (m_CachedPipelineLayout.find(key) == m_CachedPipelineLayout.end())
	{
		m_CachedPipelineLayout[key] = new PipelineLayout(this, table);;
	}
	return PipelineLayoutRef(m_CachedPipelineLayout[key]);
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

PipelineStateObjectRef
Device::NewPipelineState(rhi::PipelineDesc const & desc, rhi::PipelineLayoutRef ppl, rhi::EPipelineType type)
{
	return CreatePipelineStateObject(desc, ppl);
}

PipelineStateObjectRef
Device::CreatePipelineStateObject(rhi::PipelineDesc const & desc, rhi::PipelineLayoutRef ppl)
{
	return MakeShared<PipelineStateObject>(this, desc, static_cast<PipelineLayout*>(ppl.Get()));
}

SyncFenceRef
Device::NewFence()
{
	return MakeShared<Fence>(this);
}

GpuResourceRef
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
	return GpuResourceRef(resource);
}

ShaderResourceViewRef
Device::NewShaderResourceView(rhi::GpuResourceRef pRes, rhi::ResourceViewDesc const & desc)
{
	return MakeShared<ShaderResourceView>(desc, pRes.Get());
}

rhi::IDescriptorPool *
Device::NewDescriptorPool()
{
	return nullptr;
}

RenderViewportRef
Device::NewRenderViewport(void * winHandle, rhi::GfxSetting& setting)
{
	return MakeShared<RenderViewport>(this, winHandle, setting);
}

PtrCmdAlloc Device::NewCommandAllocator(bool transient)
{
	return CommandAllocator::CreateAllocator(m_GraphicsQueueIndex, false, this);
}

PtrSemaphore Device::NewSemaphore()
{
	return std::make_shared<Semaphore>(this);
}

void Device::QueryTextureSubResourceLayout(rhi::GpuResourceRef resource, rhi::TextureResourceSpec const & spec, rhi::SubResourceLayout * layout)
{
	K3D_ASSERT(resource && resource->GetResourceType() != rhi::EGT_Buffer);
	vkGetImageSubresourceLayout(m_Device, (VkImage)resource->GetResourceLocation(), (const VkImageSubresource*)&spec, (VkSubresourceLayout*)layout);
}

SwapChainRef
Device::NewSwapChain(rhi::GfxSetting const & setting)
{
	return k3d::MakeShared<SwapChain>(this);
}

K3D_VK_END