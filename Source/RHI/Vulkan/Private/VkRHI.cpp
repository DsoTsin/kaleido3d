#include "VkCommon.h"
#include "VkRHI.h"
#include "Public/IVkRHI.h"
#include "Private/VkEnums.h"
#include "Private/VkUtils.h"
#include "VkConfig.h"

void InitializeVulkanRHI(const char* appName, bool debug)
{
	k3d::vk::RHIRoot::Initialize(appName, debug);
}

void DestroyVulkanRHI()
{
	k3d::vk::RHIRoot::Destroy();
}

namespace k3d
{
namespace vk
{

InstanceRef			RHIRoot::s_InstanceRef;
RenderViewport *	RHIRoot::s_Vp = nullptr;

void RHIRoot::Initialize(const char * appName, bool debug)
{
	vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dynlib::GetVulkanLib().ResolveEntry("vkCreateInstance"));
	vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dynlib::GetVulkanLib().ResolveEntry("vkDestroyInstance"));
	vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dynlib::GetVulkanLib().ResolveEntry("vkGetInstanceProcAddr"));
	vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dynlib::GetVulkanLib().ResolveEntry("vkEnumeratePhysicalDevices"));
	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)dynlib::GetVulkanLib().ResolveEntry("vkEnumerateInstanceLayerProperties");
	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)dynlib::GetVulkanLib().ResolveEntry("vkEnumerateInstanceExtensionProperties");
	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceProperties");
	vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceMemoryProperties");
	vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceQueueFamilyProperties");
	vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)dynlib::GetVulkanLib().ResolveEntry("vkGetDeviceProcAddr");
	vkCreateDevice = (PFN_vkCreateDevice)dynlib::GetVulkanLib().ResolveEntry("vkCreateDevice");
	
	EnumLayersAndExts();

	// create instance
	s_InstanceRef = InstanceRef(new vk::Instance(appName, appName, debug));
}

void RHIRoot::Destroy()
{
}

void RHIRoot::SetupDebug(VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callBack)
{
	if (s_InstanceRef)
	{
		s_InstanceRef->SetupDebugging(flags, callBack);
	}
}

uint32 RHIRoot::GetHostGpuCount()
{
	if (s_InstanceRef)
	{
		return s_InstanceRef->GetHostGpuCount();
	}
	return 0;
}

GpuRef RHIRoot::GetHostGpuById(uint32 id)
{
	return s_InstanceRef->GetHostGpuByIndex(id);
}

VkInstance RHIRoot::GetInstance()
{
	return s_InstanceRef ? s_InstanceRef->m_Instance : VK_NULL_HANDLE;
}

rhi::DeviceRef RHIRoot::GetDeviceById(uint32 id)
{
	return s_InstanceRef ? s_InstanceRef->GetDeviceByIndex(id): nullptr;
}

void RHIRoot::AddViewport(RenderViewport * vp)
{
	s_Vp = vp;
}

RenderViewport * RHIRoot::GetViewport(int index)
{
	return s_Vp;
}

void RHIRoot::EnumLayersAndExts()
{
	// Enum Layers
	uint32 layerCount = 0;
	K3D_VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	if(layerCount > 0)
	{
		gVkLayerProps.Resize(layerCount);
		K3D_VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, gVkLayerProps.Data()));
	}

	// Enum Extensions
	uint32 extCount = 0;
	K3D_VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr));
	if (extCount > 0)
	{
		gVkExtProps.Resize(extCount);
		K3D_VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &extCount, gVkExtProps.Data()));
	}
	VKLOG(Info, ">> RHIRoot::EnumLayersAndExts <<\n\n"
		"=================================>> layerCount = %d.\n"
		"=================================>> extensionCount = %d.\n", layerCount, extCount);
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
	GET_DEVICE_PROC_ADDR(GetRawDevice(), CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(GetRawDevice(), QueuePresentKHR);
}

} // End NS -- vk

class VkRHI : public IVkRHI
{
public:
	VkRHI() {}
	~VkRHI() override 
	{
		Shutdown();
	}

	void Initialize(const char* appName, bool debug) override
	{
		if (!m_IsInitialized)
		{
			k3d::vk::RHIRoot::Initialize(appName, debug);
			m_IsInitialized = true;
		}
		m_ConfigDebug = debug;
	}

	rhi::DeviceRef	GetPrimaryDevice() override 
	{
		return GetDeviceById(0);
	}
	
	uint32 GetDeviceCount() override { return m_DeviceCount; }
	
	rhi::DeviceRef	GetDeviceById(uint32 id) override
	{
		return vk::RHIRoot::GetDeviceById(id);
	}
	
	void Destroy() override
	{
	}

	void Start() override
	{
		if (!m_IsInitialized)
		{
			VKLOG(Fatal, "RHI Module uninitialized!! You should call Initialize first!");
			return;
		}
	}

	void Shutdown() override
	{
		Destroy();
	}
	
	const char * Name() { return "RHI_Vulkan"; }

private:

	bool					m_IsInitialized = false;
	uint32					m_DeviceCount = 0;
	bool					m_ConfigDebug = false;
};

}

MODULE_IMPLEMENT(RHI_Vulkan, k3d::VkRHI)
