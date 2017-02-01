#include "VkCommon.h"
#include "VkRHI.h"
#include "VkUtils.h"
#include "VkConfig.h"

using namespace rhi;

K3D_VK_BEGIN

void Instance::EnumGpus()
{
	DynArray<VkPhysicalDevice> gpus;
	uint32_t gpuCount = 0;
	K3D_VK_VERIFY(vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr));
	gpus.Resize(gpuCount);
	K3D_VK_VERIFY(vkEnumeratePhysicalDevices(m_Instance, &gpuCount, gpus.Data()));
	for (auto gpu : gpus)
	{
		auto gpuRef = GpuRef(new Gpu(gpu, this));
		m_Gpus.Append(gpuRef);
		m_GpuAdapters.Append(DeviceAdapter::Ptr(new DeviceAdapter(gpuRef)));
	}
}

DeviceAdapter::DeviceAdapter(GpuRef const & gpu)
	: m_Gpu(gpu)
{
	m_pDevice = MakeShared<Device>();
	m_pDevice->Create(this, gpu->m_Inst->WithValidation());
	m_Gpu->m_Inst->AppendLogicalDevice(m_pDevice);
}

DeviceAdapter::~DeviceAdapter()
{
}

DeviceRef DeviceAdapter::GetDevice()
{
	return m_pDevice;
}

Device::Device() 
	: m_Gpu(nullptr)
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
	if (!m_CachedDescriptorSetLayout.empty())
	{
		m_CachedDescriptorSetLayout.erase(m_CachedDescriptorSetLayout.begin(),
			m_CachedDescriptorSetLayout.end());
	}
	if (!m_CachedDescriptorPool.empty())
	{
		m_CachedDescriptorPool.erase(m_CachedDescriptorPool.begin(),
			m_CachedDescriptorPool.end());
	}
	if (!m_CachedPipelineLayout.empty())
	{
		m_CachedPipelineLayout.erase(m_CachedPipelineLayout.begin(),
			m_CachedPipelineLayout.end());
	}
	m_PendingPass.~vector();
	m_ResourceManager->~ResourceManager();
	m_ContextPool->~CommandContextPool();
	//vkDestroyDevice(m_Device, nullptr);
	VKLOG(Info, "Device Destroyed .  -- %0x.", m_Device);
	m_Device = VK_NULL_HANDLE;
}

IDevice::Result
Device::Create(rhi::IDeviceAdapter* pAdapter, bool withDbg)
{
	m_Gpu = static_cast<DeviceAdapter*>(pAdapter)->m_Gpu;
	m_Device = m_Gpu->CreateLogicDevice(withDbg);
	if(m_Device)
	{
		LoadVulkan(m_Gpu->m_Inst->m_Instance, m_Device);
		if (withDbg)
		{
			RHIRoot::SetupDebug(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT 
				| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, DebugReportCallback);
		}
		m_ResourceManager = std::make_unique<ResourceManager>(this, 1024, 1024);
		m_ContextPool = std::make_unique<CommandContextPool>(this);
		m_DefCmdQueue = InitCmdQueue(VK_QUEUE_GRAPHICS_BIT, m_Gpu->m_GraphicsQueueIndex, 0);
		m_ComputeCmdQueue = InitCmdQueue(VK_QUEUE_COMPUTE_BIT, m_Gpu->m_ComputeQueueIndex, 0);
		return rhi::IDevice::DeviceFound;
	}
	return rhi::IDevice::DeviceNotFound;
}

RenderTargetRef
Device::NewRenderTarget(rhi::RenderTargetLayout const & layout)
{
	return RenderTargetRef(new RenderTarget(this, layout));
}

uint64 Device::GetMaxAllocationCount()
{
	return m_Gpu->m_Prop.limits.maxMemoryAllocationCount;
}

SpCmdQueue Device::InitCmdQueue(VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex)
{
	return std::make_shared<CommandQueue>(this, queueTypes, queueFamilyIndex, queueIndex);
}

bool Device::FindMemoryType(uint32 typeBits, VkFlags requirementsMask, uint32 *typeIndex) const
{
#ifdef max
#undef max
	*typeIndex = std::numeric_limits<std::remove_pointer<decltype(typeIndex)>::type>::max();
#endif
	auto memProp = m_Gpu->m_MemProp;
	for (uint32_t i = 0; i < memProp.memoryTypeCount; ++i) {
		if (typeBits & 0x00000001) {
			if (requirementsMask == (memProp.memoryTypes[i].propertyFlags & requirementsMask)) {
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

rhi::PipelineLayoutRef
Device::NewPipelineLayout(rhi::PipelineLayoutDesc const & table)
{
	// Hash the table parameter here, 
	// Lookup the layout by hash code
	rhi::PipelineLayoutKey key = HashPipelineLayoutDesc(table);
	if (m_CachedPipelineLayout.find(key) == m_CachedPipelineLayout.end())
	{
		auto plRef = rhi::PipelineLayoutRef(new PipelineLayout(this, table));
		m_CachedPipelineLayout.insert({ key, plRef });
	}
	return m_CachedPipelineLayout[key];
}

DescriptorAllocRef
Device::NewDescriptorAllocator(uint32 maxSets, BindingArray const & bindings)
{
	uint32 key = util::Hash32((const char*)bindings.Data(), bindings.Count() * sizeof(VkDescriptorSetLayoutBinding));
	if (m_CachedDescriptorPool.find(key) == m_CachedDescriptorPool.end())
	{
		DescriptorAllocator::Options options = {}; 
		auto descAllocRef = DescriptorAllocRef(new DescriptorAllocator(this, options, maxSets, bindings));
		m_CachedDescriptorPool.insert({ key, descAllocRef });
	}
	return m_CachedDescriptorPool[key];
}

DescriptorSetLayoutRef
Device::NewDescriptorSetLayout(BindingArray const & bindings)
{
	uint32 key = util::Hash32((const char*)bindings.Data(), bindings.Count() * sizeof(VkDescriptorSetLayoutBinding));
	if (m_CachedDescriptorSetLayout.find(key) == m_CachedDescriptorSetLayout.end()) 
	{
		auto descSetLayoutRef = DescriptorSetLayoutRef(new DescriptorSetLayout(this, bindings));
		m_CachedDescriptorSetLayout.insert({ key, descSetLayoutRef });
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
	return MakeShared<ShaderResourceView>(desc, pRes);
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
	return CommandAllocator::CreateAllocator(m_Gpu->m_GraphicsQueueIndex, transient, this);
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