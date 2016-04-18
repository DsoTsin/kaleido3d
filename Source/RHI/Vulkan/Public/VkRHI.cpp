#include "VkCommon.h"
#include "VkRHI.h"
#include "Base/vulkandebug.h"
#include "Base/vulkantools.h"

K3D_VK_BEGIN

RHIRoot::Initializer::Initializer(std::string appName, bool enableValidation)
{
	Log::Out(LogLevel::Info, "RHIRoot::Initializer", "Create Instance.");
	Init(enableValidation, appName);
}

RHIRoot::Initializer::~Initializer()
{
	vkDestroyInstance(Instance, nullptr);
	Log::Out(LogLevel::Info, "RHIRoot::Initializer", "Destroy Instance.");
}

#if _DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
	VkDebugReportFlagsEXT       flags,
	VkDebugReportObjectTypeEXT  objectType,
	uint64_t                    object,
	size_t                      location,
	int32_t                     messageCode,
	const char*                 pLayerPrefix,
	const char*                 pMessage,
	void*                       pUserData)
{
	Log::Out(LogLevel::Warn, "VkDebugLayer", "Message: %s ", pMessage);
	return VK_FALSE;
}
#endif

VkResult RHIRoot::Initializer::Init(bool enableValidation, std::string name)
{
	VkResult err;
	err = CreateInstance(enableValidation, name);
	if (err)
	{
		Log::Out(LogLevel::Fatal, "RHIRoot::Initializer", "Could not create Vulkan instance : %s.", vkTools::errorString(err).c_str());
	}
	else
	{
		Log::Out(LogLevel::Info, "RHIRoot::Initializer", "Vulkan instance created. version %d.%d.%d", VK_VERSION_MAJOR(VK_API_VERSION), VK_VERSION_MINOR(VK_API_VERSION), VK_VERSION_PATCH(VK_API_VERSION));
	}

#if _DEBUG
	/* Load VK_EXT_debug_report entry points in debug builds */
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(Instance, "vkCreateDebugReportCallbackEXT"));
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
		reinterpret_cast<PFN_vkDebugReportMessageEXT>
		(vkGetInstanceProcAddr(Instance, "vkDebugReportMessageEXT"));
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(Instance, "vkDestroyDebugReportCallbackEXT"));
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
	callbackCreateInfo.pUserData = nullptr;

	/* Register the callback */
	VkDebugReportCallbackEXT callback;
	VkResult result = vkCreateDebugReportCallbackEXT(Instance, &callbackCreateInfo, nullptr, &callback);
#endif

	uint32_t gpuCount;
	err = vkEnumeratePhysicalDevices(Instance, &gpuCount, nullptr);
	std::vector<VkPhysicalDevice> deviceList(gpuCount);
	err = vkEnumeratePhysicalDevices(Instance, &gpuCount, deviceList.data());
	Log::Out(LogLevel::Info, "RHIRoot::Initializer", "Device Count : %d", gpuCount);
	PhysicalDevices.swap(deviceList);
	return err;
}

VkResult RHIRoot::Initializer::CreateInstance(bool enableValidation, std::string name)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = name.c_str();
	appInfo.pEngineName = name.c_str();
	appInfo.apiVersion = VK_API_VERSION;
	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#if K3DPLATFORM_OS_WIN
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
	// todo : linux/android
	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	if (enabledExtensions.size() > 0)
	{
		if (enableValidation)
		{
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	if (enableValidation)
	{
		instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
		instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
	}
	return vkCreateInstance(&instanceCreateInfo, nullptr, &Instance);
}

#if _DEBUG
RHIRoot::Initializer RHIRoot::s_Impl("k3d_vk", true);
#else
RHIRoot::Initializer RHIRoot::s_Impl("k3d_vk", false);
#endif

void EnumAllDeviceAdapter(rhi::IDeviceAdapter** & adapterList, uint32* count)
{
	*count = (uint32)RHIRoot::GetPhysicDevices().size();
	adapterList = new rhi::IDeviceAdapter*[*count];
	for (size_t i = 0; i < *count; i++)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(RHIRoot::GetPhysicDevices()[i], &properties);
		adapterList[i] = new DeviceAdapter(&(RHIRoot::GetPhysicDevices()[i]));
		Log::Out(LogLevel::Info, "EnumAllDeviceAdapter", "DeviceName is %s, VendorId is %d.", properties.deviceName, properties.vendorID);
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

// Macro to get a procedure address based on a vulkan device
#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);   \
    if (fp##entrypoint == NULL)                                         \
	{																    \
        exit(1);                                                        \
    }                                                                   \
}

void SwapChain::InitProcs()
{
	GET_INSTANCE_PROC_ADDR(RHIRoot::GetInstance(), GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(RHIRoot::GetInstance(), GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(RHIRoot::GetInstance(), GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(RHIRoot::GetInstance(), GetPhysicalDeviceSurfacePresentModesKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), QueuePresentKHR);
}

K3D_VK_END

