#include "VkCommon.h"
#include "VkObjects.h"
#include "VkEnums.h"
#include "VkConfig.h"

K3D_VK_BEGIN

RenderTargetLayout::RenderTargetLayout(rhi::RenderTargetLayout const &layout)
{

}

RenderpassAttachment::RenderpassAttachment(VkFormat fmt, VkSampleCountFlagBits samples)
{
	m_Description.LoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
		.StoreOp(VK_ATTACHMENT_STORE_OP_STORE)
		.StencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
		.StencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
		.InitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		.FinalLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		.Format(fmt)
		.Samples(samples);

	m_ClearValue.color.float32[0] = 0.0f;
	m_ClearValue.color.float32[1] = 0.0f;
	m_ClearValue.color.float32[2] = 0.0f;
	m_ClearValue.color.float32[3] = 0.0f;
	m_ClearValue.depthStencil.depth = 0.0f;
	m_ClearValue.depthStencil.stencil = 0;
}

RenderpassAttachment RenderpassAttachment::CreateColor(VkFormat format, VkSampleCountFlagBits samples)
{
	RenderpassAttachment result(format);
	result.GetDescription().Samples(samples).FinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	return result;
}

RenderpassAttachment RenderpassAttachment::CreateDepthStencil(VkFormat format, VkSampleCountFlagBits samples)
{
	RenderpassAttachment result(format);
	result.GetDescription().Samples(samples).FinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	return result;
}

Subpass & Subpass::AddColorAttachment(uint32 attachmentIndex, uint32 resolveAttachmentIndex)
{
	m_ColorAttachments.push_back(attachmentIndex); m_ResolveAttachments.push_back(resolveAttachmentIndex);
	return *this;
}

Subpass & Subpass::AddDepthStencilAttachment(uint32 attachmentIndex)
{
	m_DepthStencilAttachment.push_back(attachmentIndex);
	return *this;
}

Subpass & Subpass::AddPreserveAttachment(uint32 attachmentIndex)
{
	m_PreserveAttachments.push_back(attachmentIndex);
	return *this;
}

Subpass & Subpass::AddPreserveAttachments(const std::vector<uint32>& attachmentIndices)
{
	std::copy(std::begin(attachmentIndices), std::end(attachmentIndices), std::back_inserter(m_PreserveAttachments));
	return *this;
}

SubpassDependency::SubpassDependency(uint32 srcSubpass, uint32 dstSubpass)
{
	m_Dependency = {};
	m_Dependency.srcSubpass = srcSubpass;
	m_Dependency.dstSubpass = dstSubpass;
}

SubpassDependency & SubpassDependency::SrcStageMask(VkPipelineStageFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.srcStageMask = mask;
	}
	else {
		m_Dependency.srcStageMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::DstStageMask(VkPipelineStageFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.dstStageMask = mask;
	}
	else {
		m_Dependency.dstStageMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::StageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive)
{
	return SrcStageMask(srcStageMask, exclusive).DstStageMask(dstStageMask, exclusive);
}

SubpassDependency & SubpassDependency::SrcAccessMask(VkAccessFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.srcAccessMask = mask;
	}
	else {
		m_Dependency.srcAccessMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::DstAccessMask(VkAccessFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.dstAccessMask = mask;
	}
	else {
		m_Dependency.dstAccessMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::AccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive)
{
	return SrcAccessMask(srcAccessMask).DstAccessMask(dstAccessMask);
}

RenderpassOptions::RenderpassOptions(VkFormat colorFormat, VkSampleCountFlagBits samples)
{
	AddAttachment(RenderpassAttachment::CreateColor(colorFormat, samples));
	Subpass& subPass = Subpass().AddColorAttachment(0);
	AddSubPass(subPass);
}

RenderpassOptions::RenderpassOptions(VkFormat colorFormat, VkFormat depthStencilFormat, VkSampleCountFlagBits samples)
{
	AddAttachment(RenderpassAttachment::CreateColor(colorFormat, samples));
	AddAttachment(RenderpassAttachment::CreateDepthStencil(depthStencilFormat, samples));
	Subpass subPass = Subpass().AddColorAttachment(0).AddDepthStencilAttachment(1);
	AddSubPass(subPass);
}

BufferMemoryBarrierParams::BufferMemoryBarrierParams()
{
	m_Barrier = {};
	m_Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	m_Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	m_Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

BufferMemoryBarrierParams::BufferMemoryBarrierParams(VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	: BufferMemoryBarrierParams()
{
	m_Barrier.buffer = buffer;
	m_Barrier.srcAccessMask = srcAccessMask;
	m_Barrier.dstAccessMask = srcAccessMask;

	m_SrcStageMask = srcStageMask;
	m_DstStageMask = dstStageMask;
}

ImageMemoryBarrierParams::ImageMemoryBarrierParams()
{
	m_Barrier = {};
	m_Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	m_Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	m_Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

ImageMemoryBarrierParams::ImageMemoryBarrierParams(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	: ImageMemoryBarrierParams()
{
	m_Barrier.image = image;
	m_Barrier.oldLayout = oldLayout;
	m_Barrier.newLayout = newLayout;

	m_SrcStageMask = srcStageMask;
	m_DstStageMask = dstStageMask;
}

ImageInfo ImageInfo::FromRHI(rhi::TextureDesc const & desc)
{
	ImageInfo info;
	info.Format(g_FormatTable[desc.Format])
		.MipLevel(desc.MipLevels)
		.Layers(desc.Layers)
		.Dimens(desc.Width, desc.Height, desc.Depth);
	return info;
}


std::pair<VkImageView, VkImageViewCreateInfo> ImageViewInfo::CreateColorImageView(VkDevice device, VkFormat colorFmt, VkImage colorImage, VkImageAspectFlags aspectMask)
{
	VkImageView imageView = VK_NULL_HANDLE;
	VkImageViewCreateInfo info = CreateColorImageInfo(colorFmt, colorImage);
	K3D_ASSERT(0 != info.subresourceRange.aspectMask);
	if (VK_IMAGE_ASPECT_COLOR_BIT == info.subresourceRange.aspectMask)
	{
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
	}
	K3D_VK_VERIFY(vkCreateImageView(device, &info, nullptr, &imageView));
	return std::make_pair(std::move(imageView), std::move(info));
}

std::pair<VkImageView, VkImageViewCreateInfo> ImageViewInfo::CreateDepthStencilImageView(VkDevice device, VkFormat colorFmt, VkImage colorImage, VkImageAspectFlags aspectMask)
{
	VkImageView imageView = VK_NULL_HANDLE;
	VkImageViewCreateInfo info = CreateDepthStencilImageInfo(colorFmt, colorImage);
	K3D_ASSERT(0 != info.subresourceRange.aspectMask);
	K3D_VK_VERIFY(vkCreateImageView(device, &info, nullptr, &imageView));
	return  std::make_pair(std::move(imageView), std::move(info));
}

ImageViewInfo ImageViewInfo::From(ImageInfo const & info, VkImage image)
{
	ImageViewInfo imageViewInfo;
	imageViewInfo.Image(image).ViewType(info.GuessViewType())
		.Format(info.format);
	imageViewInfo.flags = 0;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = info.arrayLayers;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = info.mipLevels;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	return imageViewInfo;
}

void * VkObjectAllocator::Allocation(void * pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return static_cast<VkObjectAllocator*>(pUserData)->Allocation(size, alignment, allocationScope);
}

void * VkObjectAllocator::Reallocation(void * pUserData, void * pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return static_cast<VkObjectAllocator*>(pUserData)->Reallocation(pOriginal, size, alignment, allocationScope);
}

void VkObjectAllocator::Free(void * pUserData, void * pMemory)
{
	return static_cast<VkObjectAllocator*>(pUserData)->Free(pMemory);
}

void * VkObjectAllocator::Allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return __k3d_malloc__(size);
}

void * VkObjectAllocator::Reallocation(void * pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return __k3d_malloc__(size);
}

void VkObjectAllocator::Free(void * pMemory)
{
	__k3d_free__(pMemory, 0);
}


CommandBufferManager::CommandBufferManager(VkDevice vkDevice, VkCommandBufferLevel bufferLevel,
	unsigned graphicsQueueIndex)
	: device(vkDevice)
	, commandBufferLevel(bufferLevel)
	, count(0)
{
	// RESET_COMMAND_BUFFER_BIT allows command buffers to be reset individually.
	VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	info.queueFamilyIndex = graphicsQueueIndex;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	K3D_VK_VERIFY(vkCreateCommandPool(device, &info, nullptr, &pool));
}

CommandBufferManager::~CommandBufferManager()
{
	vkFreeCommandBuffers(device, pool, buffers.size(), buffers.data());
	vkDestroyCommandPool(device, pool, nullptr);
}

void CommandBufferManager::BeginFrame()
{
	count = 0;
}

VkCommandBuffer CommandBufferManager::RequestCommandBuffer()
{
	// Either we recycle a previously allocated command buffer, or create a new one.
	VkCommandBuffer ret = VK_NULL_HANDLE;
	if (count < buffers.size())
	{
		ret = buffers[count++];
		K3D_VK_VERIFY(vkResetCommandBuffer(ret, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
	}
	else
	{
		VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		info.commandPool = pool;
		info.level = commandBufferLevel;
		info.commandBufferCount = 1;
		K3D_VK_VERIFY(vkAllocateCommandBuffers(device, &info, &ret));
		buffers.push_back(ret);

		count++;
	}

	return ret;
}

Gpu::Gpu(VkPhysicalDevice const& gpu, Instance* pInst)
	: m_Gpu(gpu)
	, m_Inst(pInst)
{
	vkGetPhysicalDeviceProperties(m_Gpu, &m_Prop);
	vkGetPhysicalDeviceMemoryProperties(m_Gpu, &m_MemProp);
	VKLOG(Info, "Gpu: %s", m_Prop.deviceName);
	QuerySupportQueues();
}

void Gpu::QuerySupportQueues()
{
	uint32 queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_Gpu, &queueCount, NULL);
	if (queueCount < 1)
		return;
	m_QueueProps.Resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_Gpu, &queueCount, m_QueueProps.Data());
	uint32 qId = 0;
	for (qId = 0; qId < queueCount; qId++)
	{
		if (m_QueueProps[qId].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_GraphicsQueueIndex = qId;
			VKLOG(Info, "Device::graphicsQueueIndex(%d) queueFlags(%d).", m_GraphicsQueueIndex, m_QueueProps[qId].queueFlags);
			break;
		}
	}
	for (qId = 0; qId < queueCount; qId++)
	{
		if (m_QueueProps[qId].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			m_ComputeQueueIndex = qId;
			VKLOG(Info, "Device::ComputeQueueIndex(%d).", m_ComputeQueueIndex);
			break;
		}
	}
	for (qId = 0; qId < queueCount; qId++)
	{
		if (m_QueueProps[qId].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			m_CopyQueueIndex = qId;
			VKLOG(Info, "Device::CopyQueueIndex(%d).", m_CopyQueueIndex);
			break;
		}
	}
}

Gpu::~Gpu()
{
	if (m_Gpu)
	{
		m_Gpu = VK_NULL_HANDLE;
	}
}

VkDevice Gpu::CreateLogicDevice(bool enableValidation)
{
	VkDevice device = VK_NULL_HANDLE;
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

	if (enableValidation)
	{
		deviceCreateInfo.enabledLayerCount = 1;
		deviceCreateInfo.ppEnabledLayerNames = g_ValidationLayerNames;
	}

	if (enabledExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	K3D_VK_VERIFY(vkCreateDevice(m_Gpu, &deviceCreateInfo, nullptr, &device));
	return device;
}

VkBool32 Gpu::GetSupportedDepthFormat(VkFormat * depthFormat)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(m_Gpu, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = format;
			return true;
		}
	}

	return false;
}

VkResult Gpu::GetSurfaceSupportKHR(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 * pSupported)
{
	return m_Inst->fpGetPhysicalDeviceSurfaceSupportKHR(m_Gpu, queueFamilyIndex, surface, pSupported);
}

VkResult Gpu::GetSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR * pSurfaceCapabilities)
{
	return m_Inst->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Gpu, surface, pSurfaceCapabilities);
}

VkResult Gpu::GetSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t * pSurfaceFormatCount, VkSurfaceFormatKHR * pSurfaceFormats)
{
	return m_Inst->fpGetPhysicalDeviceSurfaceFormatsKHR(m_Gpu, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult Gpu::GetSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t * pPresentModeCount, VkPresentModeKHR * pPresentModes)
{
	return m_Inst->fpGetPhysicalDeviceSurfacePresentModesKHR(m_Gpu, surface, pPresentModeCount, pPresentModes);
}


#if K3DPLATFORM_OS_WIN
#define PLATFORM_SURFACE_EXT VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(K3DPLATFORM_OS_LINUX) && !defined(K3DPLATFORM_OS_ANDROID)
#define PLATFORM_SURFACE_EXT VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(K3DPLATFORM_OS_ANDROID)
#define PLATFORM_SURFACE_EXT VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif

::k3d::DynArray<VkLayerProperties>		gVkLayerProps;
::k3d::DynArray<VkExtensionProperties>	gVkExtProps;


Instance::Instance(const::k3d::String & engineName, const::k3d::String & appName, bool enableValidation)
	: m_EnableValidation(enableValidation)
	, m_Instance(VK_NULL_HANDLE)
	, m_DebugMsgCallback(VK_NULL_HANDLE)
{
	ExtractEnabledExtsAndLayers();

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.CStr();
	appInfo.pEngineName = engineName.CStr();
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 1);
	appInfo.engineVersion = 1;
	appInfo.applicationVersion = 0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = m_EnabledExtsRaw.Count();
	instanceCreateInfo.ppEnabledExtensionNames = m_EnabledExtsRaw.Data();
	instanceCreateInfo.enabledLayerCount = m_EnabledLayersRaw.Count();
	instanceCreateInfo.ppEnabledLayerNames = m_EnabledLayersRaw.Data();

	VkResult err = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) 
	{
		VKLOG(Error, "Cannot find a compatible Vulkan installable client driver: vkCreateInstance Failure");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		VKLOG(Error, "Cannot find a specified extension library: vkCreateInstance Failure");
	}
	else 
	{
		EnumGpus();
		EnumProcs();
	}
}

Instance::~Instance()
{
	if (!m_Gpus.empty())
	{
		m_Gpus.~DynArray();
	}
	if (!m_GpuAdapters.empty())
	{
		m_GpuAdapters.~DynArray();
	}
	if (!m_LogicDevices.empty())
	{
		m_LogicDevices.~DynArray();
	}
	if (m_Instance)
	{
		//FreeDebugCallback();
		vkDestroyInstance(m_Instance, nullptr);
		VKLOG(Info, "Instance Destroyed .  -- %0x.", m_Instance);
		m_Instance = VK_NULL_HANDLE;
	}
}

void Instance::ExtractEnabledExtsAndLayers()
{
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;

	for (uint32_t i = 0; i < gVkExtProps.Count(); i++)
	{
		if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, gVkExtProps[i].extensionName))
		{
			surfaceExtFound = 1;
			m_EnabledExts.Append(VK_KHR_SURFACE_EXTENSION_NAME);
			m_EnabledExtsRaw.Append(VK_KHR_SURFACE_EXTENSION_NAME);
		}
		if (!strcmp(PLATFORM_SURFACE_EXT, gVkExtProps[i].extensionName))
		{
			platformSurfaceExtFound = 1;
			m_EnabledExts.Append(PLATFORM_SURFACE_EXT);
			m_EnabledExtsRaw.Append(PLATFORM_SURFACE_EXT);
		}
		if (m_EnableValidation)
		{
			m_EnabledExts.Append(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			m_EnabledExtsRaw.Append(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		VKLOG(Info, "available extension : %s .", gVkExtProps[i].extensionName);
	}
	if (!surfaceExtFound)
	{
		VKLOG(Error, "vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME" extension.");
	}
	if (!platformSurfaceExtFound)
	{
		VKLOG(Error, "vkEnumerateInstanceExtensionProperties failed to find the " PLATFORM_SURFACE_EXT " extension.");
	}

	if (m_EnableValidation && !gVkLayerProps.empty())
	{
		for (auto prop : gVkLayerProps)
		{
			if (strcmp(prop.layerName, g_ValidationLayerNames[0]) == 0 /*|| strcmp(prop.layerName, g_ValidationLayerNames[1])==0*/)
			{
				m_EnabledLayers.Append(prop.layerName);
				m_EnabledLayersRaw.Append(g_ValidationLayerNames[0]);
				VKLOG(Info, "enable validation layer [%s].", prop.layerName);
				break;
			}
		}
	}

}

// Macro to get a procedure address based on a vulkan instance
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk"#entrypoint); \
    if (fp##entrypoint == NULL)                                         \
	{																    \
        exit(1);                                                        \
    }                                                                   \
}

void Instance::EnumProcs()
{
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfacePresentModesKHR);
}

void Instance::AppendLogicalDevice(rhi::DeviceRef logicalDevice)
{
	m_LogicDevices.Append(logicalDevice);
}

K3D_VK_END