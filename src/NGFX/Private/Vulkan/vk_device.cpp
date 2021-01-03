#if _WIN32
#define VK_KHR_win32_surface 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include "vk_common.h"
#include "vk_commands.h"
#include "vk_format.h"

#define VULKAN_STANDARD_LAYER "VK_LAYER_LUNARG_standard_validation"

namespace vulkan {
	GpuDevice::GpuDevice(VkPhysicalDevice device, GpuFactory* factory)
        : physical_device_(device)
        , factory_(factory)
		, mem_alloc_(this)
	{
		VK_PROTO_FN_ZERO(GetDeviceQueue);
		VK_PROTO_FN_ZERO(QueueSubmit);
		VK_PROTO_FN_ZERO(QueueWaitIdle);
		VK_PROTO_FN_ZERO(DeviceWaitIdle);

		VK_PROTO_FN_ZERO(AllocateMemory);
		VK_PROTO_FN_ZERO(FreeMemory);
		VK_PROTO_FN_ZERO(MapMemory);
		VK_PROTO_FN_ZERO(UnmapMemory);
		VK_PROTO_FN_ZERO(InvalidateMappedMemoryRanges);
		VK_PROTO_FN_ZERO(FlushMappedMemoryRanges);
		VK_PROTO_FN_ZERO(GetImageMemoryRequirements);
		VK_PROTO_FN_ZERO(GetBufferMemoryRequirements);
		VK_PROTO_FN_ZERO(BindBufferMemory);
		VK_PROTO_FN_ZERO(BindImageMemory);
		VK_PROTO_FN_ZERO(CreateImage);
		VK_PROTO_FN_ZERO(DestroyImage);
		VK_PROTO_FN_ZERO(CreateImageView);
		VK_PROTO_FN_ZERO(DestroyImageView);
		VK_PROTO_FN_ZERO(CreateBuffer);
		VK_PROTO_FN_ZERO(DestroyBuffer);
		VK_PROTO_FN_ZERO(CreateSampler);
		VK_PROTO_FN_ZERO(DestroySampler);
		VK_PROTO_FN_ZERO(CreateFence);
		VK_PROTO_FN_ZERO(DestroyFence);
		VK_PROTO_FN_ZERO(WaitForFences);
		VK_PROTO_FN_ZERO(ResetFences);

		VK_PROTO_FN_ZERO(CreateRenderPass);
		VK_PROTO_FN_ZERO(DestroyRenderPass);
		VK_PROTO_FN_ZERO(CreateFramebuffer);
		VK_PROTO_FN_ZERO(DestroyFramebuffer);
		VK_PROTO_FN_ZERO(CreateShaderModule);
		VK_PROTO_FN_ZERO(DestroyShaderModule);
		VK_PROTO_FN_ZERO(CreateGraphicsPipelines);
		VK_PROTO_FN_ZERO(CreateComputePipelines);
		VK_PROTO_FN_ZERO(DestroyPipeline);
		VK_PROTO_FN_ZERO(CreatePipelineLayout);
		VK_PROTO_FN_ZERO(DestroyPipelineLayout);
		VK_PROTO_FN_ZERO(CreatePipelineCache);
		VK_PROTO_FN_ZERO(DestroyPipelineCache);

		VK_PROTO_FN_ZERO(CreateDescriptorSetLayout);
		VK_PROTO_FN_ZERO(DestroyDescriptorSetLayout);
		VK_PROTO_FN_ZERO(CreateDescriptorPool);
		VK_PROTO_FN_ZERO(DestroyDescriptorPool);
		VK_PROTO_FN_ZERO(AllocateDescriptorSets);
		VK_PROTO_FN_ZERO(FreeDescriptorSets);
		VK_PROTO_FN_ZERO(UpdateDescriptorSets);

		// ~ nv raytracing
		VK_PROTO_FN_ZERO(CreateAccelerationStructureNV);
		VK_PROTO_FN_ZERO(DestroyAccelerationStructureNV);
		VK_PROTO_FN_ZERO(GetAccelerationStructureMemoryRequirementsNV);
		VK_PROTO_FN_ZERO(CreateRayTracingPipelinesNV);
		VK_PROTO_FN_ZERO(BindAccelerationStructureMemoryNV);
		VK_PROTO_FN_ZERO(GetAccelerationStructureHandleNV);
		VK_PROTO_FN_ZERO(CmdBuildAccelerationStructureNV);
		VK_PROTO_FN_ZERO(GetRayTracingShaderGroupHandlesNV);
		VK_PROTO_FN_ZERO(CmdTraceRaysNV);
		// ~ end nv rt

		// ~ debug marker ext
		VK_PROTO_FN_ZERO(DebugMarkerSetObjectNameEXT);
		VK_PROTO_FN_ZERO(CmdDebugMarkerBeginEXT);
		VK_PROTO_FN_ZERO(CmdDebugMarkerEndEXT);
		VK_PROTO_FN_ZERO(CmdDebugMarkerInsertEXT);
		// ~ end debug marker ext

		// ~ debug util
		VK_PROTO_FN_ZERO(SetDebugUtilsObjectNameEXT);
		VK_PROTO_FN_ZERO(QueueBeginDebugUtilsLabelEXT);
		VK_PROTO_FN_ZERO(QueueInsertDebugUtilsLabelEXT);
		VK_PROTO_FN_ZERO(CmdBeginDebugUtilsLabelEXT);
		VK_PROTO_FN_ZERO(CmdInsertDebugUtilsLabelEXT);
		VK_PROTO_FN_ZERO(CmdEndDebugUtilsLabelEXT);
		// ~ end debug util

		VK_PROTO_FN_ZERO(CreateSwapchainKHR);
		VK_PROTO_FN_ZERO(AcquireNextImageKHR);
		VK_PROTO_FN_ZERO(QueuePresentKHR);
		VK_PROTO_FN_ZERO(GetSwapchainImagesKHR);
		VK_PROTO_FN_ZERO(DestroySwapchainKHR);

		VK_PROTO_FN_ZERO(AllocateCommandBuffers);
		VK_PROTO_FN_ZERO(ResetCommandBuffer);
		VK_PROTO_FN_ZERO(FreeCommandBuffers);
		VK_PROTO_FN_ZERO(BeginCommandBuffer);
		VK_PROTO_FN_ZERO(EndCommandBuffer);

		VK_PROTO_FN_ZERO(CreateCommandPool);
		VK_PROTO_FN_ZERO(ResetCommandPool);
		VK_PROTO_FN_ZERO(DestroyCommandPool);

		VK_PROTO_FN_ZERO(CmdBeginRenderPass);
		VK_PROTO_FN_ZERO(CmdEndRenderPass);
		VK_PROTO_FN_ZERO(CmdBindPipeline);
		VK_PROTO_FN_ZERO(CmdBindDescriptorSets);
		VK_PROTO_FN_ZERO(CmdBindVertexBuffers);
		VK_PROTO_FN_ZERO(CmdBindIndexBuffer);
		VK_PROTO_FN_ZERO(CmdSetViewport);
		VK_PROTO_FN_ZERO(CmdSetScissor);
		VK_PROTO_FN_ZERO(CmdSetStencilReference);
		VK_PROTO_FN_ZERO(CmdSetDepthBias);
		VK_PROTO_FN_ZERO(CmdDraw);
		VK_PROTO_FN_ZERO(CmdDrawIndexed);
		VK_PROTO_FN_ZERO(CmdDrawIndirect);
		VK_PROTO_FN_ZERO(CmdDispatch);
		VK_PROTO_FN_ZERO(CmdCopyBuffer);
		VK_PROTO_FN_ZERO(CmdCopyImage);
		VK_PROTO_FN_ZERO(CmdCopyBufferToImage);
		VK_PROTO_FN_ZERO(CmdCopyImageToBuffer);
		VK_PROTO_FN_ZERO(CmdBlitImage);
		VK_PROTO_FN_ZERO(CmdPipelineBarrier);
	}

	GpuDevice::~GpuDevice()
	{
		if (device_)
		{
			if (__DeviceWaitIdle)
				__DeviceWaitIdle(device_);
			factory_->__DestroyDevice(device_, NGFXVK_ALLOCATOR);
			device_ = VK_NULL_HANDLE;
		}
	}

	void GpuDevice::createDevice()
	{
		ExtensionProps extensions;
		factory_->getDeviceExtensions(physical_device_, extensions);

		HWProps	    properties = {};
		HWFeatures	features = {};
		QueueProps	queueProps;
		factory_->getDeviceProps(physical_device_, properties, features, queueProps);

		LayerProps layerProps;
		factory_->getDeviceLayers(physical_device_, layerProps);

		ngfx::Vec<const char*> required_extensions;
		ngfx::Vec<const char*> requiredLayers;

		if (extensions.hasExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {
			bool nonUniformIndexing = false;
			factory_->checkNonUniformIndexing(physical_device_, nonUniformIndexing);
			required_extensions.push(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		}

		if (extensions.hasExtension(VK_NV_RAY_TRACING_EXTENSION_NAME)) {
			bool nvRaytracingExists = false;
			factory_->checkNVRaytracing(physical_device_, nvRaytracingExists);
			required_extensions.push(VK_NV_RAY_TRACING_EXTENSION_NAME);
			support_raytracing_ = nvRaytracingExists;
		}

		queues_info_.graphics.queueFamilyIndex = getQueueFamilyIndex(queueProps, VK_QUEUE_GRAPHICS_BIT);
		queues_info_.compute.queueFamilyIndex = getQueueFamilyIndex(queueProps, VK_QUEUE_COMPUTE_BIT);
		queues_info_.transfer.queueFamilyIndex = getQueueFamilyIndex(queueProps, VK_QUEUE_TRANSFER_BIT);
		queues_info_.presentQueueFamilyIndex = queues_info_.graphics.queueFamilyIndex;

		ngfx::Vec<VkDeviceQueueCreateInfo> queue_infos;
		float priority = 1.0f;
		VkDeviceQueueCreateInfo queue_info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queue_info.queueFamilyIndex = queues_info_.graphics.queueFamilyIndex;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = &priority;
		queue_infos.push(queue_info);

		if (queues_info_.graphics.queueFamilyIndex != queues_info_.compute.queueFamilyIndex)
		{
			queue_info.queueFamilyIndex = queues_info_.compute.queueFamilyIndex;
			queue_infos.push(queue_info);
		}
		if (queues_info_.transfer.queueFamilyIndex != queues_info_.graphics.queueFamilyIndex &&
			queues_info_.transfer.queueFamilyIndex != queues_info_.compute.queueFamilyIndex)
		{
			queue_info.queueFamilyIndex = queues_info_.transfer.queueFamilyIndex;
			queue_infos.push(queue_info);
		}
		if (factory_->debuggable()) {
			if (extensions.hasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
				required_extensions.push(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			if (extensions.hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				required_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			if (extensions.hasExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
				required_extensions.push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			requiredLayers.push(VULKAN_STANDARD_LAYER);
		}

		if (extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			required_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		if (extensions.hasExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
			required_extensions.push(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

		VkDeviceCreateInfo device_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0,
			(uint32_t)queue_infos.num(), &queue_infos[0],
			(uint32_t)requiredLayers.num(), requiredLayers.num() > 0 ? &requiredLayers[0] : nullptr,
			(uint32_t)required_extensions.num(), required_extensions.num() > 0 ? &required_extensions[0] : nullptr,
			&features
		};
		VkResult result = factory_->__CreateDevice(physical_device_, &device_info, NGFXVK_ALLOCATOR, &device_);
		check(result == VK_SUCCESS);

		if (result == VK_SUCCESS) {
			loadDeviceFunctions();
			mem_alloc_.init();

			switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				device_type_ = ngfx::DeviceType::Discrete; break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				device_type_ = ngfx::DeviceType::Integrate; break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				device_type_ = ngfx::DeviceType::Virtual; break;
			default:
				device_type_ = ngfx::DeviceType::Integrate; break;
			}
			label_ = properties.deviceName;
		} else {
			factory_->printLogStr(-1, "failed to initialize device!\n");
		}
	}

	void GpuDevice::loadDeviceFunctions()
	{
#define VK_DEVICE_FN_RSV(name)	__##name = (PFN_vk##name)factory_->__GetDeviceProcAddr(device_, VK_FN_STRNAME(name))
		VK_DEVICE_FN_RSV(GetDeviceQueue);
		VK_DEVICE_FN_RSV(QueueSubmit);
		VK_DEVICE_FN_RSV(QueueWaitIdle);
		VK_DEVICE_FN_RSV(DeviceWaitIdle);

		VK_DEVICE_FN_RSV(AllocateMemory);
		VK_DEVICE_FN_RSV(FreeMemory);
		VK_DEVICE_FN_RSV(MapMemory);
		VK_DEVICE_FN_RSV(UnmapMemory);
		VK_DEVICE_FN_RSV(InvalidateMappedMemoryRanges);
		VK_DEVICE_FN_RSV(FlushMappedMemoryRanges);
		VK_DEVICE_FN_RSV(GetImageMemoryRequirements);
		VK_DEVICE_FN_RSV(GetBufferMemoryRequirements);
		VK_DEVICE_FN_RSV(BindBufferMemory);
		VK_DEVICE_FN_RSV(BindImageMemory);
		VK_DEVICE_FN_RSV(CreateImage);
		VK_DEVICE_FN_RSV(DestroyImage);
		VK_DEVICE_FN_RSV(CreateImageView);
		VK_DEVICE_FN_RSV(DestroyImageView);
		VK_DEVICE_FN_RSV(CreateBuffer);
		VK_DEVICE_FN_RSV(DestroyBuffer);
		VK_DEVICE_FN_RSV(CreateSampler);
		VK_DEVICE_FN_RSV(DestroySampler);
		VK_DEVICE_FN_RSV(CreateFence);
		VK_DEVICE_FN_RSV(DestroyFence);
		VK_DEVICE_FN_RSV(WaitForFences);
		VK_DEVICE_FN_RSV(ResetFences);
		VK_DEVICE_FN_RSV(CreateSemaphore);
		VK_DEVICE_FN_RSV(DestroySemaphore);

		VK_DEVICE_FN_RSV(CreateRenderPass);
		VK_DEVICE_FN_RSV(DestroyRenderPass);
		VK_DEVICE_FN_RSV(CreateFramebuffer);
		VK_DEVICE_FN_RSV(DestroyFramebuffer);
		VK_DEVICE_FN_RSV(CreateShaderModule);
		VK_DEVICE_FN_RSV(DestroyShaderModule);
		VK_DEVICE_FN_RSV(CreateGraphicsPipelines);
		VK_DEVICE_FN_RSV(CreateComputePipelines);
		VK_DEVICE_FN_RSV(DestroyPipeline);
		VK_DEVICE_FN_RSV(CreatePipelineLayout);
		VK_DEVICE_FN_RSV(DestroyPipelineLayout);
		VK_DEVICE_FN_RSV(CreatePipelineCache);
		VK_DEVICE_FN_RSV(DestroyPipelineCache);

		VK_DEVICE_FN_RSV(CreateDescriptorSetLayout);
		VK_DEVICE_FN_RSV(DestroyDescriptorSetLayout);
		VK_DEVICE_FN_RSV(CreateDescriptorPool);
		VK_DEVICE_FN_RSV(DestroyDescriptorPool);
		VK_DEVICE_FN_RSV(AllocateDescriptorSets);
		VK_DEVICE_FN_RSV(FreeDescriptorSets);
		VK_DEVICE_FN_RSV(UpdateDescriptorSets);

		VK_DEVICE_FN_RSV(CreateSwapchainKHR);
		VK_DEVICE_FN_RSV(AcquireNextImageKHR);
		VK_DEVICE_FN_RSV(QueuePresentKHR);
		VK_DEVICE_FN_RSV(GetSwapchainImagesKHR);
		VK_DEVICE_FN_RSV(DestroySwapchainKHR);

		VK_DEVICE_FN_RSV(AllocateCommandBuffers);
		VK_DEVICE_FN_RSV(ResetCommandBuffer);
		VK_DEVICE_FN_RSV(FreeCommandBuffers);
		VK_DEVICE_FN_RSV(BeginCommandBuffer);
		VK_DEVICE_FN_RSV(EndCommandBuffer);
		VK_DEVICE_FN_RSV(CreateCommandPool);
		VK_DEVICE_FN_RSV(ResetCommandPool);
		VK_DEVICE_FN_RSV(DestroyCommandPool);

		VK_DEVICE_FN_RSV(CmdBeginRenderPass);
		VK_DEVICE_FN_RSV(CmdEndRenderPass);
		VK_DEVICE_FN_RSV(CmdBindPipeline);
		VK_DEVICE_FN_RSV(CmdBindDescriptorSets);
		VK_DEVICE_FN_RSV(CmdBindVertexBuffers);
		VK_DEVICE_FN_RSV(CmdBindIndexBuffer);
		VK_DEVICE_FN_RSV(CmdSetViewport);
		VK_DEVICE_FN_RSV(CmdSetScissor);
		VK_DEVICE_FN_RSV(CmdSetStencilReference);
		VK_DEVICE_FN_RSV(CmdSetDepthBias);
		VK_DEVICE_FN_RSV(CmdDraw);
		VK_DEVICE_FN_RSV(CmdDrawIndexed);
		VK_DEVICE_FN_RSV(CmdDrawIndirect);
		VK_DEVICE_FN_RSV(CmdDispatch);
		VK_DEVICE_FN_RSV(CmdCopyBuffer);
		VK_DEVICE_FN_RSV(CmdCopyImage);
		VK_DEVICE_FN_RSV(CmdCopyBufferToImage);
		VK_DEVICE_FN_RSV(CmdCopyImageToBuffer);
		VK_DEVICE_FN_RSV(CmdBlitImage);
		VK_DEVICE_FN_RSV(CmdPipelineBarrier);

		if (support_raytracing_) {
			VK_DEVICE_FN_RSV(CreateAccelerationStructureNV);
			VK_DEVICE_FN_RSV(DestroyAccelerationStructureNV);
			VK_DEVICE_FN_RSV(GetAccelerationStructureMemoryRequirementsNV);
			VK_DEVICE_FN_RSV(CreateRayTracingPipelinesNV);
			VK_DEVICE_FN_RSV(BindAccelerationStructureMemoryNV);
			VK_DEVICE_FN_RSV(GetAccelerationStructureHandleNV);
			VK_DEVICE_FN_RSV(CmdBuildAccelerationStructureNV);
			VK_DEVICE_FN_RSV(GetRayTracingShaderGroupHandlesNV);
			VK_DEVICE_FN_RSV(CmdTraceRaysNV);
		}

		if (factory_->debuggable()) {
			VK_DEVICE_FN_RSV(DebugMarkerSetObjectNameEXT);
			VK_DEVICE_FN_RSV(CmdDebugMarkerBeginEXT);
			VK_DEVICE_FN_RSV(CmdDebugMarkerEndEXT);
			VK_DEVICE_FN_RSV(CmdDebugMarkerInsertEXT);
			VK_DEVICE_FN_RSV(SetDebugUtilsObjectNameEXT);
		}
#undef VK_DEVICE_FN_RSV
	}

    void GpuDevice::setLabel(const char * label)
    {
		if (label) label_ = label;
    }
    
	const char * GpuDevice::label() const
    {
		return label_.empty() ? nullptr : label_.c_str();
    }

    ngfx::DeviceType GpuDevice::getType() const
    {
        return device_type_;
    }

	ngfx::CommandQueue* GpuDevice::newQueue(ngfx::Result* result)
	{
        VkQueue queue = VK_NULL_HANDLE;
		uint32_t familyIdx = (uint32_t)queues_info_.graphics.queueFamilyIndex;
        this->__GetDeviceQueue(device_, familyIdx, 0, &queue);
		if (result) *result = (queue != VK_NULL_HANDLE) ? ngfx::Result::Ok : ngfx::Result::Failed;
		return new GpuQueue(queue, familyIdx, this);
	}
    
    GpuFence::GpuFence(GpuDevice * device)
        : device_(device)
    {
        VkFenceCreateInfo create_info = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
        };
        device_->createFence(create_info, &fence_);
    }

    GpuFence::~GpuFence()
    {
        device_->destroyFence(fence_);
        fence_ = VK_NULL_HANDLE;
    }

    void GpuFence::setLabel(const char* label)
    {
		if (label) label_ = label;
    }

    const char* GpuFence::label() const
    {
		return label_.empty() ? nullptr : label_.c_str();
    }

	// ---- GpuShader ----

	GpuShader::GpuShader(GpuDevice* device)
		: device_(device)
	{
	}

	GpuShader::~GpuShader()
	{
	}

	void GpuShader::setLabel(const char* label)
	{
		if (label) label_ = label;
	}

	const char* GpuShader::label() const
	{
		return label_.empty() ? nullptr : label_.c_str();
	}

	// ---- GpuRenderpass ----

	GpuRenderpass::GpuRenderpass(GpuDevice* device, VkRenderPass renderPass)
		: render_pass_(renderPass), device_(device)
	{
	}

	GpuRenderpass::~GpuRenderpass()
	{
		if (render_pass_ != VK_NULL_HANDLE) {
			device_->destroyRenderPass(render_pass_);
			render_pass_ = VK_NULL_HANDLE;
		}
	}

	ngfx::RenderPipeline* GpuRenderpass::newRenderPipeline(const ngfx::RenderPipelineDesc* desc, ngfx::Result* result)
	{
		auto* pipeline = new GpuRenderPipeline(device_.get(), *desc, render_pass_);
		if (pipeline->vkPipeline() != VK_NULL_HANDLE) {
			if (result) *result = ngfx::Result::Ok;
			return pipeline;
		}
		delete pipeline;
		if (result) *result = ngfx::Result::Failed;
		return nullptr;
	}

    bool GpuDevice::querySurfaceInfo(VkSurfaceKHR surface, GpuDevice::SurfaceInfo& info)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &count, nullptr);
        info.formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &count, &info.formats[0]);
        count = 0;
        VkSurfaceCapabilitiesKHR surface_caps = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &surface_caps);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface, &count, nullptr);
        info.present_modes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface, &count, &info.present_modes[0]);
        return true;
    }
	
    ngfx::Shader* GpuDevice::newShader()
	{
		return new GpuShader(this);
	}

	ngfx::Renderpass* GpuDevice::newRenderpass(const ngfx::RenderpassDesc * desc, ngfx::Result * result)
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkResult ret = this->createRenderPass(&renderPassInfo, &renderPass);
		if (ret == VK_SUCCESS) {
			if (result) *result = ngfx::Result::Ok;
			return new GpuRenderpass(this, renderPass);
		}
		if (result) *result = ngfx::Result::Failed;
		return nullptr;
	}

	ngfx::ComputePipeline* GpuDevice::newComputePipeline(const ngfx::ComputePipelineDesc * desc, ngfx::Result * result)
	{
		auto* pipeline = new GpuComputePipeline(this, *desc);
		if (pipeline->vkPipeline() != VK_NULL_HANDLE) {
			if (result) *result = ngfx::Result::Ok;
			return pipeline;
		}
		if (result) *result = ngfx::Result::Failed;
		return pipeline;
	}

	ngfx::Fence* GpuDevice::newFence(ngfx::Result * result)
	{
		if (result) *result = ngfx::Result::Ok;
		return new GpuFence(this);
	}

	ngfx::Result GpuDevice::wait()
	{
		if (__DeviceWaitIdle) {
			VkResult ret = __DeviceWaitIdle(device_);
			return (ret == VK_SUCCESS) ? ngfx::Result::Ok : ngfx::Result::Failed;
		}
		return ngfx::Result::Failed;
	}

	GpuAllocator& GpuDevice::getAllocator()
	{
		return mem_alloc_;
	}

	int GpuDevice::getQueueFamilyIndex(QueueProps& queueProps, VkQueueFlags queueFlag)
	{
		if (queueFlag & VK_QUEUE_COMPUTE_BIT)
		{
			for (uint32_t i = 0; i < queueProps.num(); ++i)
			{
				if ((queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
					!(queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
					return i;
			}
		}
		if (queueFlag & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < queueProps.num(); ++i)
			{
				if ((queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
					!(queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					!(queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
					return i;
			}
		}
		for (uint32_t i = 0; i < queueProps.num(); ++i)
		{
			if (queueProps[i].queueFlags & queueFlag)
				return i;
		}
		return 0;
	}

    GpuQueue::GpuQueue(VkQueue queue, uint32_t familyIndex, GpuDevice* device)
        : queue_(queue)
		, family_index_(familyIndex)
        , device_(device)
    {
    }
    
	GpuQueue::~GpuQueue()
    {
    }
}

ngfx::Factory* CreateFactory(bool debug_layer_enable, ngfx_LogCallback log_call)
{
    return new vulkan::GpuFactory(VK_NULL_HANDLE, debug_layer_enable, log_call);
}
