#include "vk_common.h"
namespace vulkan
{
	inline void GpuDevice::getPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties& memProps)
	{
		factory_->__GetPhysicalDeviceMemoryProperties(physical_device_, &memProps);
	}

	inline VkResult GpuDevice::createSwapchain(const VkSwapchainCreateInfoKHR * pCreateInfo, VkSwapchainKHR * pSwapchain)
	{
		return __CreateSwapchainKHR(device_, pCreateInfo, NGFXVK_ALLOCATOR, pSwapchain);
	}

	inline void GpuDevice::destroySwapchain(VkSwapchainKHR swapchain)
	{
		__DestroySwapchainKHR(device_, swapchain, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::getSwapchainImages(VkSwapchainKHR swapchain, uint32_t * pSwapchainImageCount, VkImage * pSwapchainImages)
	{
		return this->__GetSwapchainImagesKHR(device_, swapchain, pSwapchainImageCount, pSwapchainImages);
	}

	inline VkResult GpuDevice::acquireNextImage(VkSwapchainKHR swapchain, uint64_t timeOut, VkSemaphore semaphore, VkFence fence, uint32_t * imageIndex)
	{
		return __AcquireNextImageKHR(device_, swapchain, timeOut, semaphore, fence, imageIndex);
	}
	
	inline VkResult GpuDevice::allocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory)
	{
		return this->__AllocateMemory(device_, pAllocateInfo, NGFXVK_ALLOCATOR, pMemory);
	}

	inline void GpuDevice::freeMemory(VkDeviceMemory memory)
	{
		this->__FreeMemory(device_, memory, NGFXVK_ALLOCATOR);
	}

	inline void* GpuDevice::mapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags)
	{
		void* ptr = nullptr;
		this->__MapMemory(device_, mem, offset, size, flags, &ptr);
		return ptr;
	}

	inline void GpuDevice::flushMappedMemory(uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges)
	{
		this->__FlushMappedMemoryRanges(device_, memoryRangeCount, pMemoryRanges);
	}

	inline void GpuDevice::invalidMappedMemory(uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges)
	{
		this->__InvalidateMappedMemoryRanges(device_, memoryRangeCount, pMemoryRanges);
	}

	inline void GpuDevice::unmapMemory(VkDeviceMemory mem)
	{
		this->__UnmapMemory(device_, mem);
	}

	inline VkResult GpuDevice::createBuffer(const VkBufferCreateInfo * pCreateInfo, VkBuffer * pBuffer)
	{
		return __CreateBuffer(device_, pCreateInfo, NGFXVK_ALLOCATOR, pBuffer);
	}

	inline void GpuDevice::destroyBuffer(VkBuffer buffer)
	{
		__DestroyBuffer(device_, buffer, NGFXVK_ALLOCATOR);
	}

	inline void GpuDevice::getBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
	{
		this->__GetBufferMemoryRequirements(device_, buffer, pMemoryRequirements);
	}

	inline VkResult GpuDevice::createImage(const VkImageCreateInfo* pCreateInfo, VkImage* pImage)
	{
		return __CreateImage(device_, pCreateInfo, NGFXVK_ALLOCATOR, pImage);
	}

	inline void GpuDevice::destroyImage(VkImage image)
	{
		__DestroyImage(device_, image, NGFXVK_ALLOCATOR);
	}
	
	inline void GpuDevice::getImageMemoryRequirements(VkImage image, VkMemoryRequirements * pMemoryRequirements)
	{
		this->__GetImageMemoryRequirements(device_, image, pMemoryRequirements);
	}

	inline VkResult GpuDevice::createImageView(const VkImageViewCreateInfo* pCreateInfo, VkImageView* pView)
	{
		return __CreateImageView(device_, pCreateInfo, NGFXVK_ALLOCATOR, pView);
	}

	inline void GpuDevice::destroyImageView(VkImageView view)
	{
		__DestroyImageView(device_, view, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createSampler(const VkSamplerCreateInfo * pCreateInfo, VkSampler * pSampler)
	{
		return __CreateSampler(device_, pCreateInfo, NGFXVK_ALLOCATOR, pSampler);
	}

	inline void GpuDevice::destroySampler(VkSampler sampler)
	{
		__DestroySampler(device_, sampler, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createRenderPass(const VkRenderPassCreateInfo* pCreateInfo, VkRenderPass* pRenderPass)
	{
		return __CreateRenderPass(device_, pCreateInfo, NGFXVK_ALLOCATOR, pRenderPass);
	}

	inline void GpuDevice::destroyRenderPass(VkRenderPass renderPass)
	{
		__DestroyRenderPass(device_, renderPass, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createFramebuffer(const VkFramebufferCreateInfo* pCreateInfo, VkFramebuffer* pFramebuffer)
	{
		return __CreateFramebuffer(device_, pCreateInfo, NGFXVK_ALLOCATOR, pFramebuffer);
	}

	inline void GpuDevice::destroyFramebuffer(VkFramebuffer framebuffer)
	{
		__DestroyFramebuffer(device_, framebuffer, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, VkShaderModule* pShaderModule)
	{
		return __CreateShaderModule(device_, pCreateInfo, NGFXVK_ALLOCATOR, pShaderModule);
	}

	inline void GpuDevice::destroyShaderModule(VkShaderModule shaderModule)
	{
		__DestroyShaderModule(device_, shaderModule, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createGraphicsPipelines(VkPipelineCache cache, uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos, VkPipeline* pPipelines)
	{
		return __CreateGraphicsPipelines(device_, cache, count, pCreateInfos, NGFXVK_ALLOCATOR, pPipelines);
	}

	inline VkResult GpuDevice::createComputePipelines(VkPipelineCache cache, uint32_t count, const VkComputePipelineCreateInfo* pCreateInfos, VkPipeline* pPipelines)
	{
		return __CreateComputePipelines(device_, cache, count, pCreateInfos, NGFXVK_ALLOCATOR, pPipelines);
	}

	inline void GpuDevice::destroyPipeline(VkPipeline pipeline)
	{
		__DestroyPipeline(device_, pipeline, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createPipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, VkPipelineLayout* pPipelineLayout)
	{
		return __CreatePipelineLayout(device_, pCreateInfo, NGFXVK_ALLOCATOR, pPipelineLayout);
	}

	inline void GpuDevice::destroyPipelineLayout(VkPipelineLayout pipelineLayout)
	{
		__DestroyPipelineLayout(device_, pipelineLayout, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createPipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, VkPipelineCache* pPipelineCache)
	{
		return __CreatePipelineCache(device_, pCreateInfo, NGFXVK_ALLOCATOR, pPipelineCache);
	}

	inline void GpuDevice::destroyPipelineCache(VkPipelineCache pipelineCache)
	{
		__DestroyPipelineCache(device_, pipelineCache, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayout* pSetLayout)
	{
		return __CreateDescriptorSetLayout(device_, pCreateInfo, NGFXVK_ALLOCATOR, pSetLayout);
	}

	inline void GpuDevice::destroyDescriptorSetLayout(VkDescriptorSetLayout setLayout)
	{
		__DestroyDescriptorSetLayout(device_, setLayout, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, VkDescriptorPool* pPool)
	{
		return __CreateDescriptorPool(device_, pCreateInfo, NGFXVK_ALLOCATOR, pPool);
	}

	inline void GpuDevice::destroyDescriptorPool(VkDescriptorPool pool)
	{
		__DestroyDescriptorPool(device_, pool, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::allocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocInfo, VkDescriptorSet* pSets)
	{
		return __AllocateDescriptorSets(device_, pAllocInfo, pSets);
	}

	inline void GpuDevice::updateDescriptorSets(uint32_t writeCount, const VkWriteDescriptorSet* pWrites, uint32_t copyCount, const VkCopyDescriptorSet* pCopies)
	{
		__UpdateDescriptorSets(device_, writeCount, pWrites, copyCount, pCopies);
	}

	inline void GpuDevice::getAccelerationStructureMemoryRequirements(
		VkAccelerationStructureNV accelerationStructure,
		VkAccelerationStructureMemoryRequirementsTypeNV type,
		VkMemoryRequirements2KHR* pMemoryRequirements)
	{
		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
		memoryRequirementsInfo.type = type;
		memoryRequirementsInfo.accelerationStructure = accelerationStructure;
		if(__GetAccelerationStructureMemoryRequirementsNV)
			__GetAccelerationStructureMemoryRequirementsNV(device_, &memoryRequirementsInfo, pMemoryRequirements);
	}
	
	inline VkResult GpuDevice::createAccelerationStructure(
		const VkAccelerationStructureCreateInfoNV * pCreateInfo,
		const VkAllocationCallbacks * pAllocator,
		VkAccelerationStructureNV * pAccelerationStructure)
	{
		if (this->__CreateAccelerationStructureNV) {
			return this->__CreateAccelerationStructureNV(device_, pCreateInfo, pAllocator, pAccelerationStructure);
		}
		return VkResult::VK_ERROR_INITIALIZATION_FAILED;
	}

	inline void GpuDevice::destroyAccelerationStructure(VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks * pAllocator)
	{
		if (this->__DestroyAccelerationStructureNV) {
			this->__DestroyAccelerationStructureNV(device_, accelerationStructure, pAllocator);
		}
	}

	inline VkResult GpuDevice::createFence(const VkFenceCreateInfo & info, VkFence * pFence)
	{
		return this->__CreateFence(device_, &info, NGFXVK_ALLOCATOR, pFence);
	}

	inline void GpuDevice::destroyFence(VkFence fence)
	{
		this->__DestroyFence(device_, fence, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
	{
		return this->__WaitForFences(device_, fenceCount, pFences, waitAll, timeout);
	}

	inline VkResult GpuDevice::resetFences(uint32_t fenceCount, const VkFence* pFences)
	{
		return this->__ResetFences(device_, fenceCount, pFences);
	}

	inline VkResult GpuDevice::createSemaphore(const VkSemaphoreCreateInfo& info, VkSemaphore * pSemaphore)
	{
		return this->__CreateSemaphore(device_, &info, NGFXVK_ALLOCATOR, pSemaphore);
	}

	inline void GpuDevice::destroySemaphore(VkSemaphore semaphore)
	{
		this->__DestroySemaphore(device_, semaphore, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::createCommandPool(const VkCommandPoolCreateInfo* info, VkCommandPool* commandPool)
	{
		return this->__CreateCommandPool(device_, info, NGFXVK_ALLOCATOR, commandPool);
	}

	inline void GpuDevice::destroyCommandPool(VkCommandPool commandPool)
	{
		this->__DestroyCommandPool(device_, commandPool, NGFXVK_ALLOCATOR);
	}

	inline VkResult GpuDevice::resetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags flags)
	{
		return this->__ResetCommandPool(device_, commandPool, flags);
	}

	inline VkResult GpuDevice::allocateCommandBuffer(const VkCommandBufferAllocateInfo* info, VkCommandBuffer* cmds)
	{
		return this->__AllocateCommandBuffers(device_, info, cmds);
	}

	inline void GpuDevice::freeCommandBuffer(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
	{
		this->__FreeCommandBuffers(device_, commandPool, commandBufferCount, pCommandBuffers);
	}

	inline VkResult GpuDevice::beginCommandBuffer(VkCommandBuffer cmdBuf, const VkCommandBufferBeginInfo* pBeginInfo)
	{
		return this->__BeginCommandBuffer(cmdBuf, pBeginInfo);
	}

	inline VkResult GpuDevice::endCommandBuffer(VkCommandBuffer cmdBuf)
	{
		return this->__EndCommandBuffer(cmdBuf);
	}

	inline VkResult GpuDevice::queueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
	{
		return this->__QueueSubmit(queue, submitCount, pSubmits, fence);
	}

	inline VkResult GpuDevice::queuePresent(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
	{
		return this->__QueuePresentKHR(queue, pPresentInfo);
	}
}
