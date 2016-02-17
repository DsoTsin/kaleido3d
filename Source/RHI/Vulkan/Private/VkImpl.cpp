#include "VkCommon.h"
#include "Public/VkRHI.h"
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

VkResult RHIRoot::Initializer::Init(bool enableValidation, std::string name)
{
	VkResult err;
	err = CreateInstance(enableValidation, name);
	if (err)
	{
		Log::Out(LogLevel::Fatal, "RHIRoot::Initializer", "Could not create Vulkan instance : %s.", vkTools::errorString(err).c_str());
	}
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

RHIRoot::Initializer RHIRoot::s_Impl("k3d_vk", false);

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

K3D_VK_END