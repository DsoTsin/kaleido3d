#include "VkCommon.h"
#include "VkRHI.h"
#include "Private/VkEnums.h"
#include "Private/VkUtils.h"
#include "Public/VkConfig.h"

K3D_VK_BEGIN

std::vector<VkLayerProperties> RHIRoot::s_LayerProps;
std::vector<char*> RHIRoot::s_LayerNames;
RenderViewport * RHIRoot::s_Vp = nullptr;

#if _DEBUG
RHIRoot::Initializer RHIRoot::s_Impl("k3d_vk", true);
#else
RHIRoot::Initializer RHIRoot::s_Impl("Triangle", false);
#endif

RHIRoot::Initializer::Initializer(std::string appName, bool enableValidation)
{
	VKRHI_METHOD_TRACE
	InitVkEntries();
	EnumLayers();
	Init(enableValidation, appName);
}

void RHIRoot::Initializer::InitVkEntries()
{
	vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dynlib::GetVulkanLib().ResolveEntry("vkCreateInstance"));
	vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dynlib::GetVulkanLib().ResolveEntry("vkDestroyInstance"));
	vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dynlib::GetVulkanLib().ResolveEntry("vkEnumeratePhysicalDevices"));
	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)dynlib::GetVulkanLib().ResolveEntry("vkEnumerateInstanceLayerProperties");
	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)dynlib::GetVulkanLib().ResolveEntry("vkEnumerateInstanceExtensionProperties");
	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceProperties");
	vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceMemoryProperties");
	vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceQueueFamilyProperties");
	vkCreateDevice = (PFN_vkCreateDevice)dynlib::GetVulkanLib().ResolveEntry("vkCreateDevice");
}

void RHIRoot::AddViewport(RenderViewport * vp)
{
	s_Vp = vp;
}

RenderViewport * RHIRoot::GetViewport(int index)
{
	return s_Vp;
}

void RHIRoot::EnumLayers()
{
	uint32 layerCount = 0;
	K3D_VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	if(layerCount>0)
	{
		s_LayerProps.resize(layerCount);
		K3D_VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, s_LayerProps.data()));
	}
	VKLOG(Info, "layerCount = %d. ", layerCount);
}


VkResult RHIRoot::Initializer::Init(bool enableValidation, std::string name)
{
	VkResult err = CreateInstance(enableValidation, name);
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
		VKLOG(Error, "Cannot find a compatible Vulkan installable client driver: vkCreateInstance Failure");
	} else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
		VKLOG(Error, "Cannot find a specified extension library: vkCreateInstance Failure");
	} else {
		K3D_VK_VERIFY(err);
	}

	uint32_t gpuCount = 0;
	K3D_VK_VERIFY(vkEnumeratePhysicalDevices(Instance, &gpuCount, nullptr));
	VKLOG(Info, "RHIRoot::Initializer Device Count : %u .", gpuCount);
	std::vector<VkPhysicalDevice> deviceList(gpuCount);
	err = vkEnumeratePhysicalDevices(Instance, &gpuCount, deviceList.data());
	VkPhysicalDeviceProperties physicalDeviceProperties = {};
	vkGetPhysicalDeviceProperties(deviceList[0], &physicalDeviceProperties);
	VKLOG(Info, "Vulkan First Device: %s", physicalDeviceProperties.deviceName);
	PhysicalDevices.swap(deviceList);
	return err;
}

VkResult RHIRoot::Initializer::CreateInstance(bool enableValidation, std::string name)
{
#if K3DPLATFORM_OS_WIN
#define PLATFORM_SURFACE_EXT VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(K3DPLATFORM_OS_LINUX) && !defined(K3DPLATFORM_OS_ANDROID)
#define PLATFORM_SURFACE_EXT VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(K3DPLATFORM_OS_ANDROID)
#define PLATFORM_SURFACE_EXT VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif
	uint32_t instanceExtensionCount = 0;
	K3D_VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));
	VKLOG(Info, "extension num : %d.", instanceExtensionCount);
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
	VkExtensionProperties* instanceExtensions = new VkExtensionProperties[instanceExtensionCount];
	K3D_VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions));
	std::vector<const char*> enabledExtensions;
	for (uint32_t i = 0; i < instanceExtensionCount; i++) {
		if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName))
		{
			surfaceExtFound = 1;
			enabledExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
		}
		if (!strcmp(PLATFORM_SURFACE_EXT, instanceExtensions[i].extensionName))
		{
			platformSurfaceExtFound = 1;
			enabledExtensions.push_back( PLATFORM_SURFACE_EXT );
		}
		if (enableValidation)
		{
			enabledExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
		}
		KLOG(Info, kaleido3d::VulkanRHI, "available extension : %s .", instanceExtensions[i].extensionName);
	}
	if (!surfaceExtFound)
	{
		KLOG(Error, kaleido3d::VulkanRHI, "vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME" extension.");
	}
	if (!platformSurfaceExtFound)
	{
		KLOG(Error, kaleido3d::VulkanRHI,"vkEnumerateInstanceExtensionProperties failed to find the " PLATFORM_SURFACE_EXT " extension.");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = name.c_str();
	appInfo.pEngineName = name.c_str();
	appInfo.apiVersion = VK_MAKE_VERSION(1,0,1);
	appInfo.engineVersion = 1;
	appInfo.applicationVersion = 0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	if (enableValidation && !s_LayerProps.empty())
	{
		for (auto prop : s_LayerProps)
		{
			if (strcmp(prop.layerName, g_ValidationLayerNames[0]) == 0 || strcmp(prop.layerName, g_ValidationLayerNames[1])==0)
			{
				s_LayerNames.push_back(prop.layerName);
				instanceCreateInfo.enabledLayerCount = s_LayerNames.size();
				instanceCreateInfo.ppEnabledLayerNames = s_LayerNames.data();
				KLOG(Info, kaleido3d::VulkanRHI, "enable validation layer [%s].", prop.layerName);
				break;
			}
		}
	}
	return vkCreateInstance(&instanceCreateInfo, nullptr, &Instance);
}

void RHIRoot::Initializer::Destroy()
{
#if _DEBUG
	FreeDebugCallback(Instance);
#endif
	vkDestroyInstance(Instance, nullptr);
	Log::Out(LogLevel::Info, "RHIRoot::Initializer", "Destroy Instance.");
}

RHIRoot::Initializer::~Initializer()
{
	Destroy();
}

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