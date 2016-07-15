#include "VkCommon.h"
#include "DynVulkanLoader.h"


#ifdef K3DPLATFORM_OS_WIN
static const char* LIBVULKAN = "vulkan-1.dll";
#elif defined(K3DPLATFORM_OS_MAC)
static const char* LIBVULKAN = "libvulkan.dylib";
#else
#include <dlfcn.h>
static const char* LIBVULKAN = "libvulkan.so";
#endif


namespace dynlib
{
	Lib::Lib(const char* libName)
		: m_LibHandle(NULL)
	{
#if K3DPLATFORM_OS_WIN
		m_LibHandle = LoadLibrary(libName);
#else
		m_LibHandle = dlopen(libName, RTLD_NOW|RTLD_LOCAL);
#endif
	}

	Lib::~Lib()
	{
		if (m_LibHandle)
		{
#if K3DPLATFORM_OS_WIN
			::FreeLibrary((HMODULE)m_LibHandle);
#else
			dlclose(m_LibHandle);
#endif
			m_LibHandle = NULL;
		}
	}

	void * Lib::ResolveEntry(const char * functionName)
	{
#if K3DPLATFORM_OS_WIN
		return ::GetProcAddress((HMODULE)m_LibHandle, functionName);
#else
        return dlsym(m_LibHandle, functionName);
#endif
	}

	Lib & GetVulkanLib() {
		static Lib vkLib(LIBVULKAN);
		return vkLib;
	}
}

_DEF_VK_FUNC_(GetInstanceProcAddr);
_DEF_VK_FUNC_(GetDeviceProcAddr);

_DEF_VK_FUNC_(CreateInstance);
_DEF_VK_FUNC_(DestroyInstance);
_DEF_VK_FUNC_(EnumeratePhysicalDevices);

_DEF_VK_FUNC_(QueuePresentKHR);
_DEF_VK_FUNC_(GetPhysicalDeviceSurfaceSupportKHR);
_DEF_VK_FUNC_(DestroySurfaceKHR);
_DEF_VK_FUNC_(GetPhysicalDeviceSurfaceCapabilitiesKHR);

#if K3DPLATFORM_OS_WIN
_DEF_VK_FUNC_(CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
_DEF_VK_FUNC_(CreateAndroidSurfaceKHR);
#endif

_DEF_VK_FUNC_(GetPhysicalDeviceFeatures);
_DEF_VK_FUNC_(GetPhysicalDeviceFormatProperties);
_DEF_VK_FUNC_(GetPhysicalDeviceImageFormatProperties);
_DEF_VK_FUNC_(GetPhysicalDeviceProperties);
_DEF_VK_FUNC_(GetPhysicalDeviceQueueFamilyProperties);
_DEF_VK_FUNC_(GetPhysicalDeviceMemoryProperties);
_DEF_VK_FUNC_(CreateDevice);
_DEF_VK_FUNC_(DestroyDevice);
_DEF_VK_FUNC_(EnumerateInstanceExtensionProperties);
_DEF_VK_FUNC_(EnumerateDeviceExtensionProperties);
_DEF_VK_FUNC_(EnumerateInstanceLayerProperties);
_DEF_VK_FUNC_(EnumerateDeviceLayerProperties);
_DEF_VK_FUNC_(GetDeviceQueue);
_DEF_VK_FUNC_(QueueSubmit);
_DEF_VK_FUNC_(QueueWaitIdle);
_DEF_VK_FUNC_(DeviceWaitIdle);
_DEF_VK_FUNC_(AllocateMemory);
_DEF_VK_FUNC_(FreeMemory);
_DEF_VK_FUNC_(MapMemory);
_DEF_VK_FUNC_(UnmapMemory);
_DEF_VK_FUNC_(FlushMappedMemoryRanges);
_DEF_VK_FUNC_(InvalidateMappedMemoryRanges);
_DEF_VK_FUNC_(GetDeviceMemoryCommitment);
_DEF_VK_FUNC_(BindBufferMemory);
_DEF_VK_FUNC_(BindImageMemory);
_DEF_VK_FUNC_(GetBufferMemoryRequirements);
_DEF_VK_FUNC_(GetImageMemoryRequirements);
_DEF_VK_FUNC_(GetImageSparseMemoryRequirements);
_DEF_VK_FUNC_(GetPhysicalDeviceSparseImageFormatProperties);
_DEF_VK_FUNC_(QueueBindSparse);
_DEF_VK_FUNC_(CreateFence);
_DEF_VK_FUNC_(DestroyFence);
_DEF_VK_FUNC_(ResetFences);
_DEF_VK_FUNC_(GetFenceStatus);
_DEF_VK_FUNC_(WaitForFences);
_DEF_VK_FUNC_(CreateSemaphore);
_DEF_VK_FUNC_(DestroySemaphore);
_DEF_VK_FUNC_(CreateEvent);
_DEF_VK_FUNC_(DestroyEvent);
_DEF_VK_FUNC_(GetEventStatus);
_DEF_VK_FUNC_(SetEvent);
_DEF_VK_FUNC_(ResetEvent);
_DEF_VK_FUNC_(CreateQueryPool);
_DEF_VK_FUNC_(DestroyQueryPool);
_DEF_VK_FUNC_(GetQueryPoolResults);
_DEF_VK_FUNC_(CreateBuffer);
_DEF_VK_FUNC_(DestroyBuffer);
_DEF_VK_FUNC_(CreateBufferView);
_DEF_VK_FUNC_(DestroyBufferView);
_DEF_VK_FUNC_(CreateImage);
_DEF_VK_FUNC_(DestroyImage);
_DEF_VK_FUNC_(GetImageSubresourceLayout);
_DEF_VK_FUNC_(CreateImageView);
_DEF_VK_FUNC_(DestroyImageView);
_DEF_VK_FUNC_(CreateShaderModule);
_DEF_VK_FUNC_(DestroyShaderModule);
_DEF_VK_FUNC_(CreatePipelineCache);
_DEF_VK_FUNC_(DestroyPipelineCache);
_DEF_VK_FUNC_(GetPipelineCacheData);
_DEF_VK_FUNC_(MergePipelineCaches);
_DEF_VK_FUNC_(CreateGraphicsPipelines);
_DEF_VK_FUNC_(CreateComputePipelines);
_DEF_VK_FUNC_(DestroyPipeline);
_DEF_VK_FUNC_(CreatePipelineLayout);
_DEF_VK_FUNC_(DestroyPipelineLayout);
_DEF_VK_FUNC_(CreateSampler);
_DEF_VK_FUNC_(DestroySampler);
_DEF_VK_FUNC_(CreateDescriptorSetLayout);
_DEF_VK_FUNC_(DestroyDescriptorSetLayout);
_DEF_VK_FUNC_(CreateDescriptorPool);
_DEF_VK_FUNC_(DestroyDescriptorPool);
_DEF_VK_FUNC_(ResetDescriptorPool);
_DEF_VK_FUNC_(AllocateDescriptorSets);
_DEF_VK_FUNC_(FreeDescriptorSets);
_DEF_VK_FUNC_(UpdateDescriptorSets);
_DEF_VK_FUNC_(CreateFramebuffer);
_DEF_VK_FUNC_(DestroyFramebuffer);
_DEF_VK_FUNC_(CreateRenderPass);
_DEF_VK_FUNC_(DestroyRenderPass);
_DEF_VK_FUNC_(GetRenderAreaGranularity);
_DEF_VK_FUNC_(CreateCommandPool);
_DEF_VK_FUNC_(DestroyCommandPool);
_DEF_VK_FUNC_(ResetCommandPool);
_DEF_VK_FUNC_(AllocateCommandBuffers);
_DEF_VK_FUNC_(FreeCommandBuffers);
_DEF_VK_FUNC_(BeginCommandBuffer);
_DEF_VK_FUNC_(EndCommandBuffer);
_DEF_VK_FUNC_(ResetCommandBuffer);
_DEF_VK_FUNC_(CmdBindPipeline);
_DEF_VK_FUNC_(CmdSetViewport);
_DEF_VK_FUNC_(CmdSetScissor);
_DEF_VK_FUNC_(CmdSetLineWidth);
_DEF_VK_FUNC_(CmdSetDepthBias);
_DEF_VK_FUNC_(CmdSetBlendConstants);
_DEF_VK_FUNC_(CmdSetDepthBounds);
_DEF_VK_FUNC_(CmdSetStencilCompareMask);
_DEF_VK_FUNC_(CmdSetStencilWriteMask);
_DEF_VK_FUNC_(CmdSetStencilReference);
_DEF_VK_FUNC_(CmdBindDescriptorSets);
_DEF_VK_FUNC_(CmdBindIndexBuffer);
_DEF_VK_FUNC_(CmdBindVertexBuffers);
_DEF_VK_FUNC_(CmdDraw);
_DEF_VK_FUNC_(CmdDrawIndexed);
_DEF_VK_FUNC_(CmdDrawIndirect);
_DEF_VK_FUNC_(CmdDrawIndexedIndirect);
_DEF_VK_FUNC_(CmdDispatch);
_DEF_VK_FUNC_(CmdDispatchIndirect);
_DEF_VK_FUNC_(CmdCopyBuffer);
_DEF_VK_FUNC_(CmdCopyImage);
_DEF_VK_FUNC_(CmdBlitImage);
_DEF_VK_FUNC_(CmdCopyBufferToImage);
_DEF_VK_FUNC_(CmdCopyImageToBuffer);
_DEF_VK_FUNC_(CmdUpdateBuffer);
_DEF_VK_FUNC_(CmdFillBuffer);
_DEF_VK_FUNC_(CmdClearColorImage);
_DEF_VK_FUNC_(CmdClearDepthStencilImage);
_DEF_VK_FUNC_(CmdClearAttachments);
_DEF_VK_FUNC_(CmdResolveImage);
_DEF_VK_FUNC_(CmdSetEvent);
_DEF_VK_FUNC_(CmdResetEvent);
_DEF_VK_FUNC_(CmdWaitEvents);
_DEF_VK_FUNC_(CmdPipelineBarrier);
_DEF_VK_FUNC_(CmdBeginQuery);
_DEF_VK_FUNC_(CmdEndQuery);
_DEF_VK_FUNC_(CmdResetQueryPool);
_DEF_VK_FUNC_(CmdWriteTimestamp);
_DEF_VK_FUNC_(CmdCopyQueryPoolResults);
_DEF_VK_FUNC_(CmdPushConstants);
_DEF_VK_FUNC_(CmdBeginRenderPass);
_DEF_VK_FUNC_(CmdNextSubpass);
_DEF_VK_FUNC_(CmdEndRenderPass);
_DEF_VK_FUNC_(CmdExecuteCommands);
_DEF_VK_FUNC_(AcquireNextImageKHR);

int LoadVulkan(VkInstance instance, VkDevice device)
{
	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dynlib::GetVulkanLib().ResolveEntry("vkGetInstanceProcAddr");
	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)dynlib::GetVulkanLib().ResolveEntry("vkEnumerateInstanceLayerProperties");
	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)dynlib::GetVulkanLib().ResolveEntry("vkEnumerateInstanceExtensionProperties");
	vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceSurfaceSupportKHR");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)dynlib::GetVulkanLib().ResolveEntry("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

#if K3DPLATFORM_OS_WIN
	_VK_GET_INSTANCE_POINTER_(instance, CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
	_VK_GET_INSTANCE_POINTER_(instance, CreateAndroidSurfaceKHR);
#endif
	_VK_GET_INSTANCE_POINTER_(instance, DestroySurfaceKHR);

	_VK_GET_INSTANCE_POINTER_(instance, GetDeviceProcAddr);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceFeatures);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceFormatProperties);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceImageFormatProperties);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceProperties);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceQueueFamilyProperties);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceMemoryProperties);
	_VK_GET_INSTANCE_POINTER_(instance, CreateDevice);
	_VK_GET_INSTANCE_POINTER_(instance, EnumerateDeviceExtensionProperties);
	_VK_GET_INSTANCE_POINTER_(instance, EnumerateDeviceLayerProperties);
	_VK_GET_INSTANCE_POINTER_(instance, GetPhysicalDeviceSparseImageFormatProperties);

	_VK_GET_DEVICE_POINTER_(device, DestroyDevice);
	_VK_GET_DEVICE_POINTER_(device, GetDeviceQueue);
	_VK_GET_DEVICE_POINTER_(device, QueueSubmit);
	_VK_GET_DEVICE_POINTER_(device, QueueWaitIdle);
	_VK_GET_DEVICE_POINTER_(device, QueuePresentKHR);
	_VK_GET_DEVICE_POINTER_(device, DeviceWaitIdle);
	_VK_GET_DEVICE_POINTER_(device, AllocateMemory);
	_VK_GET_DEVICE_POINTER_(device, FreeMemory);
	_VK_GET_DEVICE_POINTER_(device, MapMemory);
	_VK_GET_DEVICE_POINTER_(device, UnmapMemory);
	_VK_GET_DEVICE_POINTER_(device, FlushMappedMemoryRanges);
	_VK_GET_DEVICE_POINTER_(device, InvalidateMappedMemoryRanges);
	_VK_GET_DEVICE_POINTER_(device, GetDeviceMemoryCommitment);
	_VK_GET_DEVICE_POINTER_(device, BindBufferMemory);
	_VK_GET_DEVICE_POINTER_(device, BindImageMemory);
	_VK_GET_DEVICE_POINTER_(device, GetBufferMemoryRequirements);
	_VK_GET_DEVICE_POINTER_(device, GetImageMemoryRequirements);
	_VK_GET_DEVICE_POINTER_(device, GetImageSparseMemoryRequirements);
	_VK_GET_DEVICE_POINTER_(device, QueueBindSparse);
	_VK_GET_DEVICE_POINTER_(device, CreateFence);
	_VK_GET_DEVICE_POINTER_(device, DestroyFence);
	_VK_GET_DEVICE_POINTER_(device, ResetFences);
	_VK_GET_DEVICE_POINTER_(device, GetFenceStatus);
	_VK_GET_DEVICE_POINTER_(device, WaitForFences);
	_VK_GET_DEVICE_POINTER_(device, CreateSemaphore);
	_VK_GET_DEVICE_POINTER_(device, DestroySemaphore);
	_VK_GET_DEVICE_POINTER_(device, CreateEvent);
	_VK_GET_DEVICE_POINTER_(device, DestroyEvent);
	_VK_GET_DEVICE_POINTER_(device, GetEventStatus);
	_VK_GET_DEVICE_POINTER_(device, SetEvent);
	_VK_GET_DEVICE_POINTER_(device, ResetEvent);
	_VK_GET_DEVICE_POINTER_(device, CreateQueryPool);
	_VK_GET_DEVICE_POINTER_(device, DestroyQueryPool);
	_VK_GET_DEVICE_POINTER_(device, GetQueryPoolResults);
	_VK_GET_DEVICE_POINTER_(device, CreateBuffer);
	_VK_GET_DEVICE_POINTER_(device, DestroyBuffer);
	_VK_GET_DEVICE_POINTER_(device, CreateBufferView);
	_VK_GET_DEVICE_POINTER_(device, DestroyBufferView);
	_VK_GET_DEVICE_POINTER_(device, CreateImage);
	_VK_GET_DEVICE_POINTER_(device, DestroyImage);
	_VK_GET_DEVICE_POINTER_(device, GetImageSubresourceLayout);
	_VK_GET_DEVICE_POINTER_(device, CreateImageView);
	_VK_GET_DEVICE_POINTER_(device, DestroyImageView);
	_VK_GET_DEVICE_POINTER_(device, CreateShaderModule);
	_VK_GET_DEVICE_POINTER_(device, DestroyShaderModule);
	_VK_GET_DEVICE_POINTER_(device, CreatePipelineCache);
	_VK_GET_DEVICE_POINTER_(device, DestroyPipelineCache);
	_VK_GET_DEVICE_POINTER_(device, GetPipelineCacheData);
	_VK_GET_DEVICE_POINTER_(device, MergePipelineCaches);
	_VK_GET_DEVICE_POINTER_(device, CreateGraphicsPipelines);
	_VK_GET_DEVICE_POINTER_(device, CreateComputePipelines);
	_VK_GET_DEVICE_POINTER_(device, DestroyPipeline);
	_VK_GET_DEVICE_POINTER_(device, CreatePipelineLayout);
	_VK_GET_DEVICE_POINTER_(device, DestroyPipelineLayout);
	_VK_GET_DEVICE_POINTER_(device, CreateSampler);
	_VK_GET_DEVICE_POINTER_(device, DestroySampler);
	_VK_GET_DEVICE_POINTER_(device, CreateDescriptorSetLayout);
	_VK_GET_DEVICE_POINTER_(device, DestroyDescriptorSetLayout);
	_VK_GET_DEVICE_POINTER_(device, CreateDescriptorPool);
	_VK_GET_DEVICE_POINTER_(device, DestroyDescriptorPool);
	_VK_GET_DEVICE_POINTER_(device, ResetDescriptorPool);
	_VK_GET_DEVICE_POINTER_(device, AllocateDescriptorSets);
	_VK_GET_DEVICE_POINTER_(device, FreeDescriptorSets);
	_VK_GET_DEVICE_POINTER_(device, UpdateDescriptorSets);
	_VK_GET_DEVICE_POINTER_(device, CreateFramebuffer);
	_VK_GET_DEVICE_POINTER_(device, DestroyFramebuffer);
	_VK_GET_DEVICE_POINTER_(device, CreateRenderPass);
	_VK_GET_DEVICE_POINTER_(device, DestroyRenderPass);
	_VK_GET_DEVICE_POINTER_(device, GetRenderAreaGranularity);
	_VK_GET_DEVICE_POINTER_(device, CreateCommandPool);
	_VK_GET_DEVICE_POINTER_(device, DestroyCommandPool);
	_VK_GET_DEVICE_POINTER_(device, ResetCommandPool);
	_VK_GET_DEVICE_POINTER_(device, AllocateCommandBuffers);
	_VK_GET_DEVICE_POINTER_(device, FreeCommandBuffers);
	_VK_GET_DEVICE_POINTER_(device, BeginCommandBuffer);
	_VK_GET_DEVICE_POINTER_(device, EndCommandBuffer);
	_VK_GET_DEVICE_POINTER_(device, ResetCommandBuffer);
	_VK_GET_DEVICE_POINTER_(device, CmdBindPipeline);
	_VK_GET_DEVICE_POINTER_(device, CmdSetViewport);
	_VK_GET_DEVICE_POINTER_(device, CmdSetScissor);
	_VK_GET_DEVICE_POINTER_(device, CmdSetLineWidth);
	_VK_GET_DEVICE_POINTER_(device, CmdSetDepthBias);
	_VK_GET_DEVICE_POINTER_(device, CmdSetBlendConstants);
	_VK_GET_DEVICE_POINTER_(device, CmdSetDepthBounds);
	_VK_GET_DEVICE_POINTER_(device, CmdSetStencilCompareMask);
	_VK_GET_DEVICE_POINTER_(device, CmdSetStencilWriteMask);
	_VK_GET_DEVICE_POINTER_(device, CmdSetStencilReference);
	_VK_GET_DEVICE_POINTER_(device, CmdBindDescriptorSets);
	_VK_GET_DEVICE_POINTER_(device, CmdBindIndexBuffer);
	_VK_GET_DEVICE_POINTER_(device, CmdBindVertexBuffers);
	_VK_GET_DEVICE_POINTER_(device, CmdDraw);
	_VK_GET_DEVICE_POINTER_(device, CmdDrawIndexed);
	_VK_GET_DEVICE_POINTER_(device, CmdDrawIndirect);
	_VK_GET_DEVICE_POINTER_(device, CmdDrawIndexedIndirect);
	_VK_GET_DEVICE_POINTER_(device, CmdDispatch);
	_VK_GET_DEVICE_POINTER_(device, CmdDispatchIndirect);
	_VK_GET_DEVICE_POINTER_(device, CmdCopyBuffer);
	_VK_GET_DEVICE_POINTER_(device, CmdCopyImage);
	_VK_GET_DEVICE_POINTER_(device, CmdBlitImage);
	_VK_GET_DEVICE_POINTER_(device, CmdCopyBufferToImage);
	_VK_GET_DEVICE_POINTER_(device, CmdCopyImageToBuffer);
	_VK_GET_DEVICE_POINTER_(device, CmdUpdateBuffer);
	_VK_GET_DEVICE_POINTER_(device, CmdFillBuffer);
	_VK_GET_DEVICE_POINTER_(device, CmdClearColorImage);
	_VK_GET_DEVICE_POINTER_(device, CmdClearDepthStencilImage);
	_VK_GET_DEVICE_POINTER_(device, CmdClearAttachments);
	_VK_GET_DEVICE_POINTER_(device, CmdResolveImage);
	_VK_GET_DEVICE_POINTER_(device, CmdSetEvent);
	_VK_GET_DEVICE_POINTER_(device, CmdResetEvent);
	_VK_GET_DEVICE_POINTER_(device, CmdWaitEvents);
	_VK_GET_DEVICE_POINTER_(device, CmdPipelineBarrier);
	_VK_GET_DEVICE_POINTER_(device, CmdBeginQuery);
	_VK_GET_DEVICE_POINTER_(device, CmdEndQuery);
	_VK_GET_DEVICE_POINTER_(device, CmdResetQueryPool);
	_VK_GET_DEVICE_POINTER_(device, CmdWriteTimestamp);
	_VK_GET_DEVICE_POINTER_(device, CmdCopyQueryPoolResults);
	_VK_GET_DEVICE_POINTER_(device, CmdPushConstants);
	_VK_GET_DEVICE_POINTER_(device, CmdBeginRenderPass);
	_VK_GET_DEVICE_POINTER_(device, CmdNextSubpass);
	_VK_GET_DEVICE_POINTER_(device, CmdEndRenderPass);
	_VK_GET_DEVICE_POINTER_(device, CmdExecuteCommands);
	_VK_GET_DEVICE_POINTER_(device, AcquireNextImageKHR);

	return 0;
}

#include <sstream>
#include <iomanip>

#ifdef K3DPLATFORM_OS_ANDROID
#include <android/log.h>
#define OutputDebugStringA(a) __android_log_print(ANDROID_LOG_INFO, "vkdebug", a)
#endif

std::string DumpSubmitInfo(VkSubmitInfo info)
{
	std::stringstream submitInfo;
	submitInfo << "\tSubmitInfo [\n\tsType=" << info.sType << ", pNext=" << info.pNext << ", \n\twaitSemaphoreCount=" << info.waitSemaphoreCount;
	if (info.waitSemaphoreCount == 0)
	{
		submitInfo << ", pWaitSemaphores=nullptr";
	}
	else
	{
		for (int i = 0; i < info.waitSemaphoreCount; ++i)
		{
			submitInfo << ", pWaitSemaphores[" << i << "]=" << std::hex << std::setfill('0') << (info.pWaitSemaphores ? info.pWaitSemaphores[i] : 0);
		}
	}
	submitInfo << ", \n\tcommandBufferCount=" << info.commandBufferCount;
	if (info.commandBufferCount == 0)
	{
		submitInfo << ", pCommandBuffers=nullptr";
	}
	else
	{
		for (int i = 0; i < info.commandBufferCount; ++i)
		{
			submitInfo << ", pCommandBuffers[" << i << "]=" << std::hex << std::setfill('0') << info.pCommandBuffers[i];
		}
	}
	submitInfo << ", \n\tsignalSemaphoreCount=" << info.signalSemaphoreCount;
	if (info.signalSemaphoreCount == 0)
	{
		submitInfo << ", pSignalSemaphores=nullptr";
	}
	else
	{
		for (int i = 0; i < info.signalSemaphoreCount; ++i)
		{
			submitInfo << ", pSignalSemaphores[" << i << "]=" << std::hex << std::setfill('0') << (info.pSignalSemaphores ? info.pSignalSemaphores[i] : 0);
		}
	}
	submitInfo << "]";
	return submitInfo.str();
}

VkResult vkCmd::QueueSubmit(VkQueue Queue, uint32 SubmitCount, const VkSubmitInfo * Submits, VkFence Fence)
{
	std::stringstream params;
	params << "vkCmd::QueueSubmit() Queue: 0x" << std::hex << std::setfill('0') << Queue << ", SubmitCount: " << SubmitCount << "\n" << DumpSubmitInfo(Submits[0]) << std::endl;
	OutputDebugStringA(params.str().c_str());
	return vkQueueSubmit(Queue, SubmitCount, Submits, Fence);
}

std::string DumpDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding SetLayoutBinding)
{
	std::stringstream param;
	param << "{ DescriptorSetLayoutBinding: binding=" << SetLayoutBinding.binding << ", descriptorType=" << SetLayoutBinding.descriptorType
		<< ", descriptorCount=" << SetLayoutBinding.descriptorCount << ", stageFlags=" << SetLayoutBinding.stageFlags << " }";
	return param.str();
}

VkResult vkCmd::CreateDescriptorSetLayout(VkDevice Device, const VkDescriptorSetLayoutCreateInfo * CreateInfo, const VkAllocationCallbacks * Allocator, VkDescriptorSetLayout * SetLayout)
{
	std::stringstream param;
	param << "vkCmd::CreateDescriptorSetLayout() device=" << std::hex << std::setfill('0') << Device << ", CreateInfo=[\n\tsType=" << CreateInfo->sType
		<< ", flags=" << CreateInfo->flags << ", bindingCount=" << CreateInfo->bindingCount;
	if (CreateInfo->bindingCount)
	{
		for (int i = 0; i < CreateInfo->bindingCount; i++)
			param << ", \n\tpBindings[" << i << "]=" << DumpDescriptorSetLayoutBinding(CreateInfo->pBindings[i]);
	}
	param << "]\n";
	OutputDebugStringA(param.str().c_str());

	return vkCreateDescriptorSetLayout(Device, CreateInfo, Allocator, SetLayout);
}

std::string DumpShaderStageCreateInfo(VkPipelineShaderStageCreateInfo pss)
{
	std::stringstream param;
	param << "{ sType=" << pss.sType << ", stage=" << pss.stage << ", flags=" << pss.flags <<
		", pName=" << pss.pName << ", module=" << std::hex << std::setfill('0') << pss.module<< " }";
	return param.str();
}

std::string DumpGraphicsPipelineCreateInfo(VkGraphicsPipelineCreateInfo gInfo)
{
	//VkStructureType                                  sType;
	//const void*                                      pNext;
	//VkPipelineCreateFlags                            flags;
	//uint32_t                                         stageCount;
	//const VkPipelineShaderStageCreateInfo*           pStages;
	//const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
	//const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
	//const VkPipelineTessellationStateCreateInfo*     pTessellationState;
	//const VkPipelineViewportStateCreateInfo*         pViewportState;
	//const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
	//const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
	//const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
	//const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
	//const VkPipelineDynamicStateCreateInfo*          pDynamicState;
	//VkPipelineLayout                                 layout;
	//VkRenderPass                                     renderPass;
	//uint32_t                                         subpass;
	//VkPipeline                                       basePipelineHandle;
	//int32_t                                          basePipelineIndex;
	std::stringstream param;
	param << "{ GraphicsPipelineCreateInfo [ sType=" << gInfo.sType << ", pNext=" << gInfo.pNext << ", flags=" << gInfo.flags << ", stageCount=" << gInfo.stageCount;
	if (gInfo.stageCount)
	{
		for (int i = 0; i < gInfo.stageCount; i++)
		{
			param << ", \n\t\tpStage[" << i << "]=" << DumpShaderStageCreateInfo(gInfo.pStages[i]);
		}
	}
	param << " }";
	return param.str();
}

VkResult vkCmd::CreateGraphicsPipelines(VkDevice Device, VkPipelineCache PipelineCache, uint32 CreateInfoCount, const VkGraphicsPipelineCreateInfo * CreateInfos, const VkAllocationCallbacks * Allocator, VkPipeline * Pipelines)
{
	std::stringstream param;
	param << "vkCmd::CreateGraphicsPipelines() device=" << std::hex << std::setfill('0') << Device << ", cache=" << std::hex << std::setfill('0') << PipelineCache << ", CreateInfoCount=" << CreateInfoCount;
	if (CreateInfoCount)
	{
		for (int i = 0; i < CreateInfoCount; i++)
			param << ", \n\tCreateInfos[" << i << "]=" << DumpGraphicsPipelineCreateInfo(CreateInfos[i]);
	}
	param << "\n";
	OutputDebugStringA(param.str().c_str());
	return vkCreateGraphicsPipelines(Device, PipelineCache, CreateInfoCount, CreateInfos, Allocator, Pipelines);
}
