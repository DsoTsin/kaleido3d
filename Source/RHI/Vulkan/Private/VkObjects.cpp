#include "VkCommon.h"
#include "VkObjects.h"
#include "VkEnums.h"
#include "VkConfig.h"
#include <Core/Os.h>

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

std::pair<VkImageView, VkImageViewCreateInfo> ImageViewInfo::CreateColorImageView(GpuRef device, VkFormat colorFmt, VkImage colorImage, VkImageAspectFlags aspectMask)
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
	K3D_VK_VERIFY(device->vkCreateImageView(device->m_LogicalDevice, &info, nullptr, &imageView));
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


CommandBufferManager::CommandBufferManager(GpuRef gpu, VkCommandBufferLevel bufferLevel,
	unsigned graphicsQueueIndex)
	: m_Gpu(gpu)
	, m_CommandBufferLevel(bufferLevel)
	, m_Count(0)
{
	// RESET_COMMAND_BUFFER_BIT allows command buffers to be reset individually.
	VkCommandPoolCreateInfo info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	info.queueFamilyIndex = graphicsQueueIndex;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	K3D_VK_VERIFY(m_Gpu->CreateCommdPool(&info, nullptr, &m_Pool));
}

CommandBufferManager::~CommandBufferManager()
{
	Destroy();
}

void CommandBufferManager::Destroy()
{
	if (!m_Pool)
		return;
	VKLOG(Info, "CommandBufferManager destroy. -- 0x%0x.", m_Pool);
	m_Gpu->FreeCommandBuffers(m_Pool, m_Buffers.size(), m_Buffers.data());
	//vkDestroyCommandPool(m_Device, m_Pool, nullptr);
	m_Pool = VK_NULL_HANDLE;
}

void CommandBufferManager::BeginFrame()
{
	m_Count = 0;
}

VkCommandBuffer CommandBufferManager::RequestCommandBuffer()
{
	// Either we recycle a previously allocated command buffer, or create a new one.
	VkCommandBuffer ret = VK_NULL_HANDLE;
	if (m_Count < m_Buffers.size())
	{
		ret = m_Buffers[m_Count++];
		K3D_VK_VERIFY(vkResetCommandBuffer(ret, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
	}
	else
	{
		VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		info.commandPool = m_Pool;
		info.level = m_CommandBufferLevel;
		info.commandBufferCount = 1;
		K3D_VK_VERIFY(m_Gpu->AllocateCommandBuffers(&info, &ret));
		m_Buffers.push_back(ret);

		m_Count++;
	}

	return ret;
}

Gpu::Gpu(VkPhysicalDevice const& gpu, Instance* pInst)
	: m_Inst(pInst)
	, m_LogicalDevice(VK_NULL_HANDLE)
	, m_PhysicalGpu(gpu)
{
	m_Inst->fpGetPhysicalDeviceProperties(m_PhysicalGpu, &m_Prop);
	m_Inst->fpGetPhysicalDeviceMemoryProperties(m_PhysicalGpu, &m_MemProp);
	VKLOG(Info, "Gpu: %s", m_Prop.deviceName);
	QuerySupportQueues();
}

void Gpu::QuerySupportQueues()
{
	uint32 queueCount = 0;
	m_Inst->fpGetPhysicalDeviceQueueFamilyProperties(m_PhysicalGpu, &queueCount, NULL);
	if (queueCount < 1)
		return;
	m_QueueProps.Resize(queueCount);
	m_Inst->fpGetPhysicalDeviceQueueFamilyProperties(m_PhysicalGpu, &queueCount, m_QueueProps.Data());
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

#define __VK_GET_DEVICE_PROC__(name, getDeviceProc, device) \
if(!vk##name) \
{\
vk##name = (PFN_vk##name)getDeviceProc(device, "vk" K3D_STRINGIFY(name)); \
if (!vk##name) \
{\
VKLOG(Fatal, "LoadDeviceProcs::" K3D_STRINGIFY(name) " not exist!" );\
exit(-1);\
}\
}

void Gpu::LoadDeviceProcs()
{
	if (!fpDestroyDevice)
	{
		fpDestroyDevice = (PFN_vkDestroyDevice)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkDestroyDevice");
	}
	if (!fpFreeCommandBuffers)
	{
		fpFreeCommandBuffers = (PFN_vkFreeCommandBuffers)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkFreeCommandBuffers");
	}
	if (!fpCreateCommandPool)
	{
		fpCreateCommandPool = (PFN_vkCreateCommandPool)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkCreateCommandPool");
	}
	if (!fpAllocateCommandBuffers)
	{
		fpAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)m_Inst->fpGetDeviceProcAddr(m_LogicalDevice, "vkAllocateCommandBuffers");
	}
	__VK_GET_DEVICE_PROC__(DestroyDevice, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetDeviceQueue, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(QueueSubmit, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(QueueWaitIdle, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(QueuePresentKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DeviceWaitIdle, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(AllocateMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(FreeMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(MapMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(UnmapMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(FlushMappedMemoryRanges, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(InvalidateMappedMemoryRanges, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetDeviceMemoryCommitment, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(BindBufferMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(BindImageMemory, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetBufferMemoryRequirements, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetImageMemoryRequirements, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetImageSparseMemoryRequirements, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(QueueBindSparse, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateFence, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyFence, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(ResetFences, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetFenceStatus, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(WaitForFences, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateSemaphore, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroySemaphore, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetEventStatus, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(SetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(ResetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateQueryPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyQueryPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetQueryPoolResults, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateBufferView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyBufferView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetImageSubresourceLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateImageView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyImageView, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateShaderModule, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyShaderModule, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreatePipelineCache, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyPipelineCache, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetPipelineCacheData, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(MergePipelineCaches, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateGraphicsPipelines, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateComputePipelines, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyPipeline, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreatePipelineLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyPipelineLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateSampler, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroySampler, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateDescriptorSetLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyDescriptorSetLayout, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateDescriptorPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyDescriptorPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(ResetDescriptorPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(AllocateDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(FreeDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(UpdateDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateFramebuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyFramebuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetRenderAreaGranularity, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateCommandPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroyCommandPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(ResetCommandPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(AllocateCommandBuffers, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(FreeCommandBuffers, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(BeginCommandBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(EndCommandBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(ResetCommandBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBindPipeline, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetViewport, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetScissor, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetLineWidth, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetDepthBias, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetBlendConstants, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetDepthBounds, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetStencilCompareMask, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetStencilWriteMask, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetStencilReference, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBindDescriptorSets, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBindIndexBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBindVertexBuffers, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdDraw, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdDrawIndexed, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdDrawIndirect, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdDrawIndexedIndirect, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdDispatch, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdDispatchIndirect, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdCopyBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdCopyImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBlitImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdCopyBufferToImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdCopyImageToBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdUpdateBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdFillBuffer, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdClearColorImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdClearDepthStencilImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdClearAttachments, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdResolveImage, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdSetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdResetEvent, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdWaitEvents, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdPipelineBarrier, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBeginQuery, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdEndQuery, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdResetQueryPool, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdWriteTimestamp, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdCopyQueryPoolResults, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdPushConstants, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdBeginRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdNextSubpass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdEndRenderPass, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CmdExecuteCommands, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(AcquireNextImageKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(CreateSwapchainKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(DestroySwapchainKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
	__VK_GET_DEVICE_PROC__(GetSwapchainImagesKHR, m_Inst->fpGetDeviceProcAddr, m_LogicalDevice);
}

Gpu::~Gpu()
{
	if (m_PhysicalGpu)
	{
		m_PhysicalGpu = VK_NULL_HANDLE;
	}

	fpDestroyDevice = nullptr;
}

VkDevice Gpu::CreateLogicDevice(bool enableValidation)
{
	if (!m_LogicalDevice)
	{
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
		K3D_VK_VERIFY(m_Inst->fpCreateDevice(m_PhysicalGpu, &deviceCreateInfo, nullptr, &m_LogicalDevice));

		LoadDeviceProcs();
	}
	return m_LogicalDevice;
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
		m_Inst->fpGetPhysicalDeviceFormatProperties(m_PhysicalGpu, format, &formatProps);
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
	return m_Inst->fpGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalGpu, queueFamilyIndex, surface, pSupported);
}

VkResult Gpu::GetSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR * pSurfaceCapabilities)
{
	return m_Inst->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalGpu, surface, pSurfaceCapabilities);
}

VkResult Gpu::GetSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t * pSurfaceFormatCount, VkSurfaceFormatKHR * pSurfaceFormats)
{
	return m_Inst->fpGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalGpu, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult Gpu::GetSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t * pPresentModeCount, VkPresentModeKHR * pPresentModes)
{
	return m_Inst->fpGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalGpu, surface, pPresentModeCount, pPresentModes);
}

void Gpu::DestroyDevice()
{
	if (fpDestroyDevice)
	{
		fpDestroyDevice(m_LogicalDevice, nullptr);
	}
}

void Gpu::FreeCommandBuffers(VkCommandPool pool, uint32 count, VkCommandBuffer * cmd)
{
	if (fpFreeCommandBuffers)
	{
		fpFreeCommandBuffers(m_LogicalDevice, pool, count, cmd);
	}
}

VkResult Gpu::CreateCommdPool(const VkCommandPoolCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkCommandPool * pCommandPool)
{
	if (fpCreateCommandPool)
	{
		return fpCreateCommandPool(m_LogicalDevice, pCreateInfo, pAllocator, pCommandPool);
	}
	return VK_NOT_READY;
}

VkResult Gpu::AllocateCommandBuffers(const VkCommandBufferAllocateInfo * pAllocateInfo, VkCommandBuffer * pCommandBuffers)
{
	return fpAllocateCommandBuffers(m_LogicalDevice, pAllocateInfo, pCommandBuffers);
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
	LoadGlobalProcs();
	EnumExtsAndLayers();
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

	VkResult err = gpCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
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
		LoadInstanceProcs();
		EnumGpus();
	}
}

Instance::~Instance()
{
	VKLOG(Info, "Instance Destroying...  -- %0x. (tid:%d)", m_Instance, Os::Thread::GetId());
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

#define __VK_GLOBAL_PROC_GET__(name, functor) gp##name = reinterpret_cast<PFN_vk##name>(functor("vk" K3D_STRINGIFY(name)))

void Instance::LoadGlobalProcs()
{
#ifdef K3DPLATFORM_OS_WIN
	static const char* LIBVULKAN = "vulkan-1.dll";
#elif defined(K3DPLATFORM_OS_MAC)
	static const char* LIBVULKAN = "libvulkan.dylib";
#else
	static const char* LIBVULKAN = "libvulkan.so";
#endif
	m_VulkanLib = MakeShared<dynlib::Lib>(LIBVULKAN);
	// load global functions
	__VK_GLOBAL_PROC_GET__(CreateInstance, m_VulkanLib->ResolveEntry);
	__VK_GLOBAL_PROC_GET__(EnumerateInstanceExtensionProperties, m_VulkanLib->ResolveEntry);
	__VK_GLOBAL_PROC_GET__(EnumerateInstanceLayerProperties, m_VulkanLib->ResolveEntry);
	__VK_GLOBAL_PROC_GET__(GetInstanceProcAddr, m_VulkanLib->ResolveEntry);
}

void Instance::EnumExtsAndLayers()
{
	uint32 layerCount = 0;
	K3D_VK_VERIFY(gpEnumerateInstanceLayerProperties(&layerCount, nullptr));
	if (layerCount > 0)
	{
		gVkLayerProps.Resize(layerCount);
		K3D_VK_VERIFY(gpEnumerateInstanceLayerProperties(&layerCount, gVkLayerProps.Data()));
	}
	uint32 extCount = 0;
	K3D_VK_VERIFY(gpEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr));
	if (extCount > 0)
	{
		gVkExtProps.Resize(extCount);
		K3D_VK_VERIFY(gpEnumerateInstanceExtensionProperties(nullptr, &extCount, gVkExtProps.Data()));
	}
	VKLOG(Info, ">> Instance::EnumLayersAndExts <<\n\n"
		"=================================>> layerCount = %d.\n"
		"=================================>> extensionCount = %d.\n", layerCount, extCount);
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

void Instance::LoadInstanceProcs()
{
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfacePresentModesKHR);

	GET_INSTANCE_PROC_ADDR(m_Instance, EnumeratePhysicalDevices);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceFeatures);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceProperties);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceMemoryProperties);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceFormatProperties);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceQueueFamilyProperties);
	GET_INSTANCE_PROC_ADDR(m_Instance, CreateDevice);
	GET_INSTANCE_PROC_ADDR(m_Instance, GetDeviceProcAddr);
	GET_INSTANCE_PROC_ADDR(m_Instance, DestroyInstance);


#if K3DPLATFORM_OS_WIN
	GET_INSTANCE_PROC_ADDR(m_Instance, CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
	GET_INSTANCE_PROC_ADDR(m_Instance, CreateAndroidSurfaceKHR);
#endif
	GET_INSTANCE_PROC_ADDR(m_Instance, DestroySurfaceKHR);
}

void Instance::AppendLogicalDevice(rhi::DeviceRef logicalDevice)
{
	m_LogicDevices.Append(logicalDevice);
}

K3D_VK_END