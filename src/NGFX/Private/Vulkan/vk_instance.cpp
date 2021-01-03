#include "vk_common.h"

#define VULKAN_STANDARD_LAYER "VK_LAYER_KHRONOS_validation"

namespace vulkan
{
	GpuFactory::GpuFactory(VkInstance instance, bool debug_enable, ngfx_LogCallback log_call)
		: soloader_("vulkan-1.dll")
		, instance_(instance)
		, debug_report_callback_(VK_NULL_HANDLE)
		, debug_enable_(debug_enable)
		, log_call_(log_call)
	{
		VK_PROTO_FN_ZERO(GetInstanceProcAddr);
		VK_PROTO_FN_ZERO(EnumeratePhysicalDevices);
		VK_PROTO_FN_ZERO(EnumerateInstanceLayerProperties);
		VK_PROTO_FN_ZERO(EnumerateInstanceExtensionProperties);

		VK_PROTO_FN_ZERO(CreateInstance);
		VK_PROTO_FN_ZERO(DestroyInstance);
		VK_PROTO_FN_ZERO(CreateDevice);
		VK_PROTO_FN_ZERO(DestroyDevice);
		VK_PROTO_FN_ZERO(GetDeviceProcAddr);

		VK_PROTO_FN_ZERO(GetPhysicalDeviceQueueFamilyProperties);
		VK_PROTO_FN_ZERO(EnumerateDeviceLayerProperties);
		VK_PROTO_FN_ZERO(EnumerateDeviceExtensionProperties);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceFeatures);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceFeatures2);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceProperties);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceProperties2);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceMemoryProperties);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceMemoryProperties2);

		//~ surface functions
#if _WIN32
		VK_PROTO_FN_ZERO(CreateWin32SurfaceKHR);
#elif defined(__ANDROID__)
		VK_PROTO_FN_ZERO(CreateAndroidSurfaceKHR);
#endif
		VK_PROTO_FN_ZERO(DestroySurfaceKHR);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceSurfaceSupportKHR);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceSurfaceCapabilitiesKHR);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceSurfaceFormatsKHR);
		VK_PROTO_FN_ZERO(GetPhysicalDeviceSurfacePresentModesKHR);
		VK_PROTO_FN_ZERO(SetHdrMetadataEXT);

		VK_PROTO_FN_ZERO(CreateDebugReportCallbackEXT);
		VK_PROTO_FN_ZERO(DestroyDebugReportCallbackEXT);

#if defined(VK_KHR_win32_surface)
		VK_PROTO_FN_ZERO(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif
		//~ end surface functions 

		loadInstanceFunctions();

		enumerateExtensions();

		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr,
			"ngfx_vulkan", 1,
			"ngfx", 1,
			VK_API_VERSION_1_1
		};
		VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr,
			0, &appInfo
		};
		ngfx::Vec<const char*> required_layers;
		ngfx::Vec<const char*> required_extensions;
		if (debug_enable_) {
			if (hasLayer(VULKAN_STANDARD_LAYER)) {
				required_layers.push(VULKAN_STANDARD_LAYER);
			}
			if (hasExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
				required_extensions.push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			}
			if (hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
				required_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}
#if _WIN32
		if (hasExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		}
#elif defined(__ANDROID__)
		if (hasExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
		}
#endif
		if (hasExtension(VK_KHR_SURFACE_EXTENSION_NAME)) {
			required_extensions.push(VK_KHR_SURFACE_EXTENSION_NAME);
		}
		// hdr
		if (hasExtension(VK_EXT_HDR_METADATA_EXTENSION_NAME)) {
			required_extensions.push(VK_EXT_HDR_METADATA_EXTENSION_NAME);
		}
		if (hasExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
			required_extensions.push(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
		}
		instanceInfo.enabledLayerCount = (uint32_t)required_layers.num();
		instanceInfo.ppEnabledLayerNames = required_layers.num() > 0 ? &required_layers[0] : nullptr;
		instanceInfo.enabledExtensionCount = (uint32_t)required_extensions.num();
		instanceInfo.ppEnabledExtensionNames = &required_extensions[0];

		VkResult result = __CreateInstance(&instanceInfo, NGFXVK_ALLOCATOR, &instance_);
		check(result == VK_SUCCESS);

		resolveInstanceFunctions();

		if (debug_enable_ && __CreateDebugReportCallbackEXT)
		{
			VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
			dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			dbgCreateInfo.pNext = NULL;
			dbgCreateInfo.pfnCallback = debugReport;
			dbgCreateInfo.pUserData = this;
			dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			__CreateDebugReportCallbackEXT(instance_,
				&dbgCreateInfo,
				NGFXVK_ALLOCATOR,
				&debug_report_callback_);
		}

		initDevices();
	}

	VkBool32 GpuFactory::debugReport(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object, size_t location, int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		GpuFactory* f = (GpuFactory*)pUserData;
		return f->report(
			flags, objectType, object,
			location, messageCode, pLayerPrefix,
			pMessage);
	}

	VkBool32 GpuFactory::report(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage)
	{
		return VK_TRUE;
	}

	void GpuFactory::loadInstanceFunctions()
	{
		VK_FN_RSV(GetInstanceProcAddr);
		VK_FN_RSV(CreateInstance);
		VK_FN_RSV(DestroyInstance);
		VK_FN_RSV(EnumeratePhysicalDevices);
		VK_FN_RSV(EnumerateInstanceLayerProperties);
		VK_FN_RSV(EnumerateInstanceExtensionProperties);

		VK_FN_RSV(GetPhysicalDeviceQueueFamilyProperties);
		VK_FN_RSV(EnumerateDeviceLayerProperties);
		VK_FN_RSV(EnumerateDeviceExtensionProperties);
		VK_FN_RSV(GetPhysicalDeviceFeatures);
		VK_FN_RSV(GetPhysicalDeviceProperties);
		VK_FN_RSV(GetPhysicalDeviceSurfaceSupportKHR);
		VK_FN_RSV(GetPhysicalDeviceSurfaceCapabilitiesKHR);
		VK_FN_RSV(GetPhysicalDeviceSurfaceFormatsKHR);
		VK_FN_RSV(GetPhysicalDeviceSurfacePresentModesKHR);

		VK_FN_RSV(DestroySurfaceKHR);
	}

	void GpuFactory::resolveInstanceFunctions()
	{
		VK_INST_FN_RSV(CreateDevice);
		VK_INST_FN_RSV(DestroyDevice);
		VK_INST_FN_RSV(GetPhysicalDeviceFeatures2);
		VK_INST_FN_RSV(GetPhysicalDeviceProperties2);
		VK_INST_FN_RSV(GetPhysicalDeviceMemoryProperties);
		VK_INST_FN_RSV(GetPhysicalDeviceMemoryProperties2);

		VK_INST_FN_RSV(GetDeviceProcAddr);

#if defined(VK_KHR_win32_surface)
		VK_INST_FN_RSV(CreateWin32SurfaceKHR);
		VK_INST_FN_RSV(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif

		VK_INST_FN_RSV(SetHdrMetadataEXT);
		VK_INST_FN_RSV(CreateDebugReportCallbackEXT);
		VK_INST_FN_RSV(DestroyDebugReportCallbackEXT);

		//VK_INST_FN_RSV(GetPhysicalDeviceSurfaceSupportKHR);
		//VK_INST_FN_RSV(GetPhysicalDeviceSurfaceCapabilitiesKHR);
		//VK_INST_FN_RSV(GetPhysicalDeviceSurfaceFormatsKHR);
		//VK_INST_FN_RSV(GetPhysicalDeviceSurfacePresentModesKHR);
	}

	GpuFactory::~GpuFactory()
	{
		// destroy devices firstly
		devices_.clear();

		if (debug_enable_ && __DestroyDebugReportCallbackEXT)
		{
			__DestroyDebugReportCallbackEXT(instance_, debug_report_callback_, NGFXVK_ALLOCATOR);
			debug_report_callback_ = VK_NULL_HANDLE;
		}

		if (instance_ != VK_NULL_HANDLE)
		{
			__DestroyInstance(instance_, NGFXVK_ALLOCATOR);
			instance_ = VK_NULL_HANDLE;
		}
	}

	void GpuFactory::initDevices()
	{
		enumPhysicalDevices();

		for (auto physical_device : physical_devices_) {
			auto device = new GpuDevice(physical_device, this);
			device->createDevice();
			if (device->isValid()) {
				iptr<GpuDevice> ptr_device(device);
				devices_.push(ptr_device);
			}
		}
	}

	void GpuFactory::enumerateExtensions()
	{
		// list layers
		uint32_t num_layers = 0;
		__EnumerateInstanceLayerProperties(&num_layers, nullptr);
		ngfx::Vec<VkLayerProperties> layer_props;
		if (num_layers > 0) {
			layer_props.resize(num_layers);
			__EnumerateInstanceLayerProperties(&num_layers, &layer_props[0]);
			for (auto& layer_prop : layer_props) {
				layer_props_.push({ layer_prop.layerName, layer_prop });
			}
		}

		// list default instance extensions
		uint32_t num_props = 0;
		__EnumerateInstanceExtensionProperties(nullptr, &num_props, nullptr);
		ngfx::Vec<VkExtensionProperties> ext_props;
		if (num_props > 0) {
			ext_props.resize(num_props);
			__EnumerateInstanceExtensionProperties(nullptr, &num_props, &ext_props[0]);
			for (auto& ext_prop : ext_props) {
				ext_props_.push({ ext_prop.extensionName, ext_prop });
			}
		}
	}

	void GpuFactory::getDeviceProps(VkPhysicalDevice device,
		VkPhysicalDeviceProperties& deviceProps,
		VkPhysicalDeviceFeatures& deviceFeatures,
		ngfx::Vec<VkQueueFamilyProperties>& queueProps)
	{
		__GetPhysicalDeviceFeatures(device, &deviceFeatures);
		__GetPhysicalDeviceProperties(device, &deviceProps);
		uint32_t queue_family_count;
		__GetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		queueProps.resize(queue_family_count);
		__GetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, &queueProps[0]);
	}

	void GpuFactory::getDeviceExtensions(VkPhysicalDevice device, ngfx::Vec<VkExtensionProperties>& extProps)
	{
		uint32_t num_prop = 0;
		__EnumerateDeviceExtensionProperties(device, nullptr, &num_prop, nullptr);
		if (num_prop > 0) {
			extProps.resize(num_prop);
			__EnumerateDeviceExtensionProperties(device, nullptr, &num_prop, &extProps[0]);
		}
	}

	void GpuFactory::getDeviceLayers(VkPhysicalDevice device, ngfx::Vec<VkLayerProperties>& layerProps)
	{
		uint32_t num_dev_layers = 0;
		__EnumerateDeviceLayerProperties(device, &num_dev_layers, nullptr);
		if (num_dev_layers > 0) {
			layerProps.resize(num_dev_layers);
			__EnumerateDeviceLayerProperties(device, &num_dev_layers, &layerProps[0]);
		}
	}

	bool LayerProps::hasLayer(const char* layerName) const
	{
		if (!layerName || strlen(layerName) == 0) return false;
		for (size_t index = 0; index < num(); index++)
		{
			if (!strcmp(this->at(index).layerName, layerName))
				return true;
		}
		return false;
	}

	bool ExtensionProps::hasExtension(const char* extName) const
	{
		if (!extName || strlen(extName) == 0) return false;
		for (size_t index = 0; index < num(); index++)
		{
			if (!strcmp(this->at(index).extensionName, extName))
				return true;
		}
		return false;
	}

	int GpuFactory::numDevices()
	{
		return (int)devices_.num();
	}
	ngfx::Device* GpuFactory::getDevice(ngfx::uint32 id)
	{
		return devices_[id].get();
	}

	void GpuFactory::enumPhysicalDevices()
	{
		uint32_t num_devs = 0;
		__EnumeratePhysicalDevices(instance_, &num_devs, nullptr);
		if (num_devs > 0) {
			physical_devices_.resize(num_devs);
			__EnumeratePhysicalDevices(instance_, &num_devs, &physical_devices_[0]);
		}
	}

	bool GpuFactory::hasLayer(std::string const& layer_name) const
	{
		return layer_props_.contains(layer_name);
	}

	bool GpuFactory::hasExtension(std::string const& extension_name) const
	{
		return ext_props_.contains(extension_name);
	}

	void GpuFactory::checkNonUniformIndexing(VkPhysicalDevice physical_device, bool& nonUniformIndex)
	{
		nonUniformIndex = false;
		if (__GetPhysicalDeviceFeatures2) {
			VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexing = { };
			descriptorIndexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
			VkPhysicalDeviceFeatures2 features2 = {};
			features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features2.pNext = &descriptorIndexing;
			__GetPhysicalDeviceFeatures2(physical_device, &features2);
			nonUniformIndex = true;
		}
	}

	void GpuFactory::checkNVRaytracing(VkPhysicalDevice physical_device, bool& nvRaytracing)
	{
		nvRaytracing = false;
		if (__GetPhysicalDeviceProperties2) {
			VkPhysicalDeviceRayTracingPropertiesNV ray_tracing_props = {};
			ray_tracing_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
			ray_tracing_props.pNext = nullptr;
			ray_tracing_props.maxRecursionDepth = 0;
			ray_tracing_props.shaderGroupHandleSize = 0;
			VkPhysicalDeviceProperties2 props;
			props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			props.pNext = &ray_tracing_props;
			props.properties = { };
			__GetPhysicalDeviceProperties2(physical_device, &props);
			nvRaytracing = true;
		}
	}
}