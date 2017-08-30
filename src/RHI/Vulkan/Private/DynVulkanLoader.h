#pragma once

#ifndef __DynVulkanLoader_h__
#define __DynVulkanLoader_h__

//#define VK_NO_PROTOTYPES
#if K3DPLATFORM_OS_WIN
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif K3DPLATFORM_OS_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR 1
#elif K3DPLATFORM_OS_LINUX
#define VK_USE_PLATFORM_XLIB_KHR 1
#endif
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <Core/KTL/SharedPtr.hpp>

#ifdef VK_NO_PROTOTYPES

#define _VK_GET_INSTANCE_POINTER_(instance, funcName) vk##funcName = (PFN_vk##funcName)vkGetInstanceProcAddr(instance, "vk" K3D_STRINGIFY(funcName));

// Macro to get a procedure address based on a vulkan instance
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) gpGetInstanceProcAddr(inst, "vk"#entrypoint); \
    if (fp##entrypoint == NULL)                                         \
	{																    \
        exit(1);                                                        \
    }                                                                   \
}

#define _VK_GET_DEVICE_POINTER_(device, funcName) vk##funcName = (PFN_vk##funcName)vkGetDeviceProcAddr(device, "vk" K3D_STRINGIFY(funcName));
#define _DEF_VK_FUNC_(funcName) PFN_vk##funcName vk##funcName = NULL
#define _PREDEF_VK_FUNC_(funcName) extern K3D_CORE_API PFN_vk##funcName vk##funcName

#if K3DPLATFORM_OS_WIN
#undef CreateSemaphore
#undef CreateEvent
#endif

_PREDEF_VK_FUNC_(GetInstanceProcAddr);
_PREDEF_VK_FUNC_(GetDeviceProcAddr);

_PREDEF_VK_FUNC_(CreateInstance);
_PREDEF_VK_FUNC_(DestroyInstance);
_PREDEF_VK_FUNC_(EnumeratePhysicalDevices);

_PREDEF_VK_FUNC_(QueuePresentKHR);
_PREDEF_VK_FUNC_(GetPhysicalDeviceSurfaceSupportKHR);
_PREDEF_VK_FUNC_(DestroySurfaceKHR);
_PREDEF_VK_FUNC_(GetPhysicalDeviceSurfaceCapabilitiesKHR);

#if K3DPLATFORM_OS_WIN
_PREDEF_VK_FUNC_(CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
_PREDEF_VK_FUNC_(CreateAndroidSurfaceKHR);
#endif

_PREDEF_VK_FUNC_(GetPhysicalDeviceFeatures);
_PREDEF_VK_FUNC_(GetPhysicalDeviceFormatProperties);
_PREDEF_VK_FUNC_(GetPhysicalDeviceImageFormatProperties);
_PREDEF_VK_FUNC_(GetPhysicalDeviceProperties);
_PREDEF_VK_FUNC_(GetPhysicalDeviceQueueFamilyProperties);
_PREDEF_VK_FUNC_(GetPhysicalDeviceMemoryProperties);
_PREDEF_VK_FUNC_(CreateDevice);
_PREDEF_VK_FUNC_(DestroyDevice);
_PREDEF_VK_FUNC_(EnumerateInstanceExtensionProperties);
_PREDEF_VK_FUNC_(EnumerateDeviceExtensionProperties);
_PREDEF_VK_FUNC_(EnumerateInstanceLayerProperties);
_PREDEF_VK_FUNC_(EnumerateDeviceLayerProperties);
_PREDEF_VK_FUNC_(GetDeviceQueue);
_PREDEF_VK_FUNC_(QueueSubmit);
_PREDEF_VK_FUNC_(QueueWaitIdle);
_PREDEF_VK_FUNC_(DeviceWaitIdle);
_PREDEF_VK_FUNC_(AllocateMemory);
_PREDEF_VK_FUNC_(FreeMemory);
_PREDEF_VK_FUNC_(MapMemory);
_PREDEF_VK_FUNC_(UnmapMemory);
_PREDEF_VK_FUNC_(FlushMappedMemoryRanges);
_PREDEF_VK_FUNC_(InvalidateMappedMemoryRanges);
_PREDEF_VK_FUNC_(GetDeviceMemoryCommitment);
_PREDEF_VK_FUNC_(BindBufferMemory);
_PREDEF_VK_FUNC_(BindImageMemory);
_PREDEF_VK_FUNC_(GetBufferMemoryRequirements);
_PREDEF_VK_FUNC_(GetImageMemoryRequirements);
_PREDEF_VK_FUNC_(GetImageSparseMemoryRequirements);
_PREDEF_VK_FUNC_(GetPhysicalDeviceSparseImageFormatProperties);
_PREDEF_VK_FUNC_(QueueBindSparse);
_PREDEF_VK_FUNC_(CreateFence);
_PREDEF_VK_FUNC_(DestroyFence);
_PREDEF_VK_FUNC_(ResetFences);
_PREDEF_VK_FUNC_(GetFenceStatus);
_PREDEF_VK_FUNC_(WaitForFences);
_PREDEF_VK_FUNC_(CreateSemaphore);
_PREDEF_VK_FUNC_(DestroySemaphore);
_PREDEF_VK_FUNC_(CreateEvent);
_PREDEF_VK_FUNC_(DestroyEvent);
_PREDEF_VK_FUNC_(GetEventStatus);
_PREDEF_VK_FUNC_(SetEvent);
_PREDEF_VK_FUNC_(ResetEvent);
_PREDEF_VK_FUNC_(CreateQueryPool);
_PREDEF_VK_FUNC_(DestroyQueryPool);
_PREDEF_VK_FUNC_(GetQueryPoolResults);
_PREDEF_VK_FUNC_(CreateBuffer);
_PREDEF_VK_FUNC_(DestroyBuffer);
_PREDEF_VK_FUNC_(CreateBufferView);
_PREDEF_VK_FUNC_(DestroyBufferView);
_PREDEF_VK_FUNC_(CreateImage);
_PREDEF_VK_FUNC_(DestroyImage);
_PREDEF_VK_FUNC_(GetImageSubresourceLayout);
_PREDEF_VK_FUNC_(CreateImageView);
_PREDEF_VK_FUNC_(DestroyImageView);
_PREDEF_VK_FUNC_(CreateShaderModule);
_PREDEF_VK_FUNC_(DestroyShaderModule);
_PREDEF_VK_FUNC_(CreatePipelineCache);
_PREDEF_VK_FUNC_(DestroyPipelineCache);
_PREDEF_VK_FUNC_(GetPipelineCacheData);
_PREDEF_VK_FUNC_(MergePipelineCaches);
_PREDEF_VK_FUNC_(CreateGraphicsPipelines);
_PREDEF_VK_FUNC_(CreateComputePipelines);
_PREDEF_VK_FUNC_(DestroyPipeline);
_PREDEF_VK_FUNC_(CreatePipelineLayout);
_PREDEF_VK_FUNC_(DestroyPipelineLayout);
_PREDEF_VK_FUNC_(CreateSampler);
_PREDEF_VK_FUNC_(DestroySampler);
_PREDEF_VK_FUNC_(CreateDescriptorSetLayout);
_PREDEF_VK_FUNC_(DestroyDescriptorSetLayout);
_PREDEF_VK_FUNC_(CreateDescriptorPool);
_PREDEF_VK_FUNC_(DestroyDescriptorPool);
_PREDEF_VK_FUNC_(ResetDescriptorPool);
_PREDEF_VK_FUNC_(AllocateDescriptorSets);
_PREDEF_VK_FUNC_(FreeDescriptorSets);
_PREDEF_VK_FUNC_(UpdateDescriptorSets);
_PREDEF_VK_FUNC_(CreateFramebuffer);
_PREDEF_VK_FUNC_(DestroyFramebuffer);
_PREDEF_VK_FUNC_(CreateRenderPass);
_PREDEF_VK_FUNC_(DestroyRenderPass);
_PREDEF_VK_FUNC_(GetRenderAreaGranularity);
_PREDEF_VK_FUNC_(CreateCommandPool);
_PREDEF_VK_FUNC_(DestroyCommandPool);
_PREDEF_VK_FUNC_(ResetCommandPool);
_PREDEF_VK_FUNC_(AllocateCommandBuffers);
_PREDEF_VK_FUNC_(FreeCommandBuffers);
_PREDEF_VK_FUNC_(BeginCommandBuffer);
_PREDEF_VK_FUNC_(EndCommandBuffer);
_PREDEF_VK_FUNC_(ResetCommandBuffer);
_PREDEF_VK_FUNC_(CmdBindPipeline);
_PREDEF_VK_FUNC_(CmdSetViewport);
_PREDEF_VK_FUNC_(CmdSetScissor);
_PREDEF_VK_FUNC_(CmdSetLineWidth);
_PREDEF_VK_FUNC_(CmdSetDepthBias);
_PREDEF_VK_FUNC_(CmdSetBlendConstants);
_PREDEF_VK_FUNC_(CmdSetDepthBounds);
_PREDEF_VK_FUNC_(CmdSetStencilCompareMask);
_PREDEF_VK_FUNC_(CmdSetStencilWriteMask);
_PREDEF_VK_FUNC_(CmdSetStencilReference);
_PREDEF_VK_FUNC_(CmdBindDescriptorSets);
_PREDEF_VK_FUNC_(CmdBindIndexBuffer);
_PREDEF_VK_FUNC_(CmdBindVertexBuffers);
_PREDEF_VK_FUNC_(CmdDraw);
_PREDEF_VK_FUNC_(CmdDrawIndexed);
_PREDEF_VK_FUNC_(CmdDrawIndirect);
_PREDEF_VK_FUNC_(CmdDrawIndexedIndirect);
_PREDEF_VK_FUNC_(CmdDispatch);
_PREDEF_VK_FUNC_(CmdDispatchIndirect);
_PREDEF_VK_FUNC_(CmdCopyBuffer);
_PREDEF_VK_FUNC_(CmdCopyImage);
_PREDEF_VK_FUNC_(CmdBlitImage);
_PREDEF_VK_FUNC_(CmdCopyBufferToImage);
_PREDEF_VK_FUNC_(CmdCopyImageToBuffer);
_PREDEF_VK_FUNC_(CmdUpdateBuffer);
_PREDEF_VK_FUNC_(CmdFillBuffer);
_PREDEF_VK_FUNC_(CmdClearColorImage);
_PREDEF_VK_FUNC_(CmdClearDepthStencilImage);
_PREDEF_VK_FUNC_(CmdClearAttachments);
_PREDEF_VK_FUNC_(CmdResolveImage);
_PREDEF_VK_FUNC_(CmdSetEvent);
_PREDEF_VK_FUNC_(CmdResetEvent);
_PREDEF_VK_FUNC_(CmdWaitEvents);
_PREDEF_VK_FUNC_(CmdPipelineBarrier);
_PREDEF_VK_FUNC_(CmdBeginQuery);
_PREDEF_VK_FUNC_(CmdEndQuery);
_PREDEF_VK_FUNC_(CmdResetQueryPool);
_PREDEF_VK_FUNC_(CmdWriteTimestamp);
_PREDEF_VK_FUNC_(CmdCopyQueryPoolResults);
_PREDEF_VK_FUNC_(CmdPushConstants);
_PREDEF_VK_FUNC_(CmdBeginRenderPass);
_PREDEF_VK_FUNC_(CmdNextSubpass);
_PREDEF_VK_FUNC_(CmdEndRenderPass);
_PREDEF_VK_FUNC_(CmdExecuteCommands);
_PREDEF_VK_FUNC_(AcquireNextImageKHR);

#endif

namespace dynlib
{
	typedef void(*CallBack)(void* pUserData);
	class Lib
	{
	public:
		Lib(const char* libName = nullptr);
		~Lib();

		void*		ResolveEntry(const char* functionName);
		void		SetDestroyCallBack(void *userData, CallBack callback);

	private:
		void*		m_pUserData;
		void*		m_LibHandle;
		CallBack	m_CallBack;
	};
	using LibRef = ::k3d::SharedPtr<Lib>;
}

extern int LoadVulkan(VkInstance instance, VkDevice device);

namespace vkCmd
{
	 VkResult  QueueSubmit(VkQueue Queue, uint32 SubmitCount, const VkSubmitInfo* Submits, VkFence Fence);
	 VkResult  CreateDescriptorSetLayout(VkDevice Device, const VkDescriptorSetLayoutCreateInfo* CreateInfo, const VkAllocationCallbacks* Allocator, VkDescriptorSetLayout* SetLayout);
	 VkResult  CreateGraphicsPipelines(VkDevice Device, VkPipelineCache PipelineCache, uint32 CreateInfoCount, const VkGraphicsPipelineCreateInfo* CreateInfos, const VkAllocationCallbacks* Allocator, VkPipeline* Pipelines);
	 VkResult  BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
	 void	   CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
}


#endif