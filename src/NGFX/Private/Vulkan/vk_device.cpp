#if _WIN32
#define VK_KHR_win32_surface 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include "vk_common.h"
#include "vk_commands.h"
#include <unordered_map>

#define VULKAN_STANDARD_LAYER "VK_LAYER_LUNARG_standard_validation"

namespace vulkan {
	GpuDevice::GpuDevice(VkPhysicalDevice device, GpuFactory* factory)
        : physical_device_(device)
        , factory_(factory)
		, mem_alloc_(this)
	{
		VK_PROTO_FN_ZERO(GetDeviceQueue);
		VK_PROTO_FN_ZERO(QueueSubmit);

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
		VK_PROTO_FN_ZERO(CreateBuffer);
		VK_PROTO_FN_ZERO(DestroyBuffer);
		VK_PROTO_FN_ZERO(CreateFence);
		VK_PROTO_FN_ZERO(DestroyFence);

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
	}

	GpuDevice::~GpuDevice()
	{
		if (device_)
		{
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
			if (nonUniformIndexing) {
			}
		}

		if (extensions.hasExtension(VK_NV_RAY_TRACING_EXTENSION_NAME)) {
			bool nvRaytracingExists = false;
			factory_->checkNVRaytracing(physical_device_, nvRaytracingExists);
			required_extensions.push(VK_NV_RAY_TRACING_EXTENSION_NAME);
			if (nvRaytracingExists) {
				support_raytracing_ = nvRaytracingExists;
			}
		}

		queues_info_.graphics.queueFamilyIndex = getQueueFamilyIndex(queueProps, VK_QUEUE_GRAPHICS_BIT);
		queues_info_.compute.queueFamilyIndex = getQueueFamilyIndex(queueProps, VK_QUEUE_COMPUTE_BIT);
		queues_info_.transfer.queueFamilyIndex = getQueueFamilyIndex(queueProps, VK_QUEUE_TRANSFER_BIT);

		queues_info_.presentQueueFamilyIndex = queues_info_.graphics.queueFamilyIndex;

		ngfx::Vec<VkDeviceQueueCreateInfo> queue_infos;
		float priority = 0.0f;
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
			if (extensions.hasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
				required_extensions.push(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			}
			if (extensions.hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
				required_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			if (extensions.hasExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
				required_extensions.push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}
			requiredLayers.push(VULKAN_STANDARD_LAYER);
		}

		if (extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}
		if (extensions.hasExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		}
#if _WIN32
		if (extensions.hasExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		}
#elif defined(__ANDROID__)
		if (extensions.hasExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	}
#endif
		if (extensions.hasExtension(VK_EXT_HDR_METADATA_EXTENSION_NAME)) {
			required_extensions.push(VK_EXT_HDR_METADATA_EXTENSION_NAME);
		}
		if (extensions.hasExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
			required_extensions.push(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
		}

		VkDeviceCreateInfo device_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0,
			(uint32_t)queue_infos.num(), &queue_infos[0],// queueCreateInfoCount queueCreateInfo
			(uint32_t)requiredLayers.num(), &requiredLayers[0],// layerCount, layer
			(uint32_t)required_extensions.num(), &required_extensions[0],// extensionCount, extensions
			&features// features
		};
		VkResult result = factory_->__CreateDevice(physical_device_, &device_info, NGFXVK_ALLOCATOR, &device_);
		check(result == VK_SUCCESS);

		if (result == VK_SUCCESS) {
			loadDeviceFunctions();
			mem_alloc_.init();
		} else {
			factory_->printLogStr(-1, "failed to initialize device!\n");
		}
	}

	void GpuDevice::loadDeviceFunctions()
	{
#define VK_DEVICE_FN_RSV(name)	__##name = (PFN_vk##name)factory_->__GetDeviceProcAddr(device_, VK_FN_STRNAME(name))
		VK_DEVICE_FN_RSV(GetDeviceQueue);
		VK_DEVICE_FN_RSV(QueueSubmit);

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
		VK_DEVICE_FN_RSV(CreateBuffer);
		VK_DEVICE_FN_RSV(DestroyBuffer);
		VK_DEVICE_FN_RSV(CreateFence);
		VK_DEVICE_FN_RSV(DestroyFence);
		VK_DEVICE_FN_RSV(CreateSemaphore);
		VK_DEVICE_FN_RSV(DestroySemaphore);

		VK_DEVICE_FN_RSV(CreateSwapchainKHR);
		VK_DEVICE_FN_RSV(AcquireNextImageKHR);
		VK_DEVICE_FN_RSV(QueuePresentKHR);
		VK_DEVICE_FN_RSV(GetSwapchainImagesKHR);
		VK_DEVICE_FN_RSV(DestroySwapchainKHR);

		VK_DEVICE_FN_RSV(AllocateCommandBuffers);
		VK_DEVICE_FN_RSV(ResetCommandBuffer);
		VK_DEVICE_FN_RSV(FreeCommandBuffers);
		VK_DEVICE_FN_RSV(CreateCommandPool);
		VK_DEVICE_FN_RSV(ResetCommandPool);
		VK_DEVICE_FN_RSV(DestroyCommandPool);

		// ~ nv raytracing
		if (support_raytracing_)
		{
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
		// ~ end nv rt

		if (factory_->debuggable()) {
			VK_DEVICE_FN_RSV(DebugMarkerSetObjectNameEXT);
			VK_DEVICE_FN_RSV(CmdDebugMarkerBeginEXT);
			VK_DEVICE_FN_RSV(CmdDebugMarkerEndEXT);
			VK_DEVICE_FN_RSV(CmdDebugMarkerInsertEXT);
		}
#undef VK_DEVICE_FN_RSV
	}

    void GpuDevice::setLabel(const char * label)
    {
    }
    
	const char * GpuDevice::label() const
    {
        return nullptr;
    }

    ngfx::DeviceType GpuDevice::getType() const
    {
        return device_type_;
    }

	ngfx::CommandQueue* GpuDevice::newQueue(ngfx::Result* result)
	{
        VkQueue queue = VK_NULL_HANDLE;
        this->__GetDeviceQueue(device_, 0, 0, &queue);
		return new GpuQueue(queue, this);
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
    }

    const char* GpuFence::label() const
    {
        return nullptr;
    }

    bool GpuDevice::querySurfaceInfo(VkSurfaceKHR surface, GpuDevice::SurfaceInfo& info)
    {
        //VkBool32 supportPresent;
        //vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, 
        //    queues_info.graphics.queueFamilyIndex, 
        //    surface, 
        //    &supportPresent);
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
		return nullptr;
	}

	ngfx::Renderpass* GpuDevice::newRenderpass(const ngfx::RenderpassDesc * desc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::ComputePipeline* GpuDevice::newComputePipeline(const ngfx::ComputePipelineDesc * desc, ngfx::Result * result)
	{
		return nullptr;
	}
	ngfx::Fence* GpuDevice::newFence(ngfx::Result * result)
	{
		return new GpuFence(this);
	}
	ngfx::Result GpuDevice::wait()
	{
		return ngfx::Result();
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
				{
					return i;
				}
			}
		}
		if (queueFlag & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < queueProps.num(); ++i)
			{
				if ((queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
					!(queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					!(queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					return i;
				}
			}
		}
		for (uint32_t i = 0; i < queueProps.num(); ++i)
		{
			if (queueProps[i].queueFlags & queueFlag)
			{
				return i;
			}
		}
		return 0;
	}

    GpuQueue::GpuQueue(VkQueue queue, GpuDevice* device)
        : queue_(queue)
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