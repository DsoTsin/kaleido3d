#include "VkCommon.h"
#include "Public/VkConfig.h"

K3D_VK_BEGIN

_DEF_VK_FUNC_(CreateDebugReportCallbackEXT);
_DEF_VK_FUNC_(DestroyDebugReportCallbackEXT);

PFN_vkDebugReportMessageEXT dbgBreakCallback;

static VkDebugReportCallbackEXT msgCallback = NULL;

const char* DebugLevelString(VkDebugReportFlagsEXT lev)
{
	switch (lev)
	{
#define STR(r) case VK_DEBUG_REPORT_ ##r ##_BIT_EXT: return #r
		STR(INFORMATION);
		STR(WARNING);
		STR(PERFORMANCE_WARNING);
		STR(ERROR);
		STR(DEBUG);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags, 
	VkDebugReportObjectTypeEXT objectType, 
	uint64_t object, size_t location, int32_t messageCode, 
	const char * pLayerPrefix,
	const char * pMessage, void * pUserData)
{
#if K3DPLATFORM_OS_WIN
	static char msg[4096] = { 0 };
	snprintf(msg, 4096, "[%s]\t%s\t[location] %d \n", pLayerPrefix, pMessage, location);
//	VKLOG(Debug, "[%s]\t%s\t[location] %d ", pLayerPrefix, pMessage, location);
	OutputDebugStringA(msg);
#endif
	return VK_TRUE;
}

void SetupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
{
	VKRHI_METHOD_TRACE
	_VK_GET_FUNCTION_FROM_LIB_(CreateDebugReportCallbackEXT);
	if (!vkCreateDebugReportCallbackEXT) 
		_VK_GET_INSTANCE_POINTER_(instance, CreateDebugReportCallbackEXT);
	_VK_GET_FUNCTION_FROM_LIB_(DestroyDebugReportCallbackEXT);
	if (!vkDestroyDebugReportCallbackEXT) 
		_VK_GET_INSTANCE_POINTER_(instance, DestroyDebugReportCallbackEXT);

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pNext = NULL;
	dbgCreateInfo.pfnCallback = &DebugReportCallback;
	dbgCreateInfo.pUserData = NULL;
	dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	K3D_VK_VERIFY(vkCreateDebugReportCallbackEXT(
		instance,
		&dbgCreateInfo,
		NULL,
		&msgCallback));
}

void FreeDebugCallback(VkInstance instance)
{
	if (msgCallback != NULL)
	{
		vkDestroyDebugReportCallbackEXT(instance, msgCallback, nullptr);
		KLOG(Info, kaleido3d::VulkanRHI, "free debug callback..");
	}
}

namespace DebugMarker
{
	bool active = false;

	PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert = VK_NULL_HANDLE;

	void setup(VkDevice device)
	{
		pfnDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
		pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
		pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
		pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
		pfnCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");

		// Set flag if at least one function pointer is present
		active = (pfnDebugMarkerSetObjectName != VK_NULL_HANDLE);
	}

	void setObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char *name)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (pfnDebugMarkerSetObjectName)
		{
			VkDebugMarkerObjectNameInfoEXT nameInfo = {};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = objectType;
			nameInfo.object = object;
			nameInfo.pObjectName = name;
			pfnDebugMarkerSetObjectName(device, &nameInfo);
		}
	}

	void setObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (pfnDebugMarkerSetObjectTag)
		{
			VkDebugMarkerObjectTagInfoEXT tagInfo = {};
			tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
			tagInfo.objectType = objectType;
			tagInfo.object = object;
			tagInfo.tagName = name;
			tagInfo.tagSize = tagSize;
			tagInfo.pTag = tag;
			pfnDebugMarkerSetObjectTag(device, &tagInfo);
		}
	}

	void beginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, kMath::Vec4f color)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (pfnCmdDebugMarkerBegin)
		{
			VkDebugMarkerMarkerInfoEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
			markerInfo.pMarkerName = pMarkerName;
			pfnCmdDebugMarkerBegin(cmdbuffer, &markerInfo);
		}
	}

	void insert(VkCommandBuffer cmdbuffer, std::string markerName, kMath::Vec4f color)
	{
		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (pfnCmdDebugMarkerInsert)
		{
			VkDebugMarkerMarkerInfoEXT markerInfo = {};
			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
			markerInfo.pMarkerName = markerName.c_str();
			pfnCmdDebugMarkerInsert(cmdbuffer, &markerInfo);
		}
	}

	void endRegion(VkCommandBuffer cmdBuffer)
	{
		// Check for valid function (may not be present if not runnin in a debugging application)
		if (pfnCmdDebugMarkerEnd)
		{
			pfnCmdDebugMarkerEnd(cmdBuffer);
		}
	}

	void setCommandBufferName(VkDevice device, VkCommandBuffer cmdBuffer, const char * name)
	{
		setObjectName(device, (uint64_t)cmdBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
	}

	void setQueueName(VkDevice device, VkQueue queue, const char * name)
	{
		setObjectName(device, (uint64_t)queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name);
	}

	void setImageName(VkDevice device, VkImage image, const char * name)
	{
		setObjectName(device, (uint64_t)image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
	}

	void setSamplerName(VkDevice device, VkSampler sampler, const char * name)
	{
		setObjectName(device, (uint64_t)sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
	}

	void setBufferName(VkDevice device, VkBuffer buffer, const char * name)
	{
		setObjectName(device, (uint64_t)buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
	}

	void setDeviceMemoryName(VkDevice device, VkDeviceMemory memory, const char * name)
	{
		setObjectName(device, (uint64_t)memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name);
	}

	void setShaderModuleName(VkDevice device, VkShaderModule shaderModule, const char * name)
	{
		setObjectName(device, (uint64_t)shaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
	}

	void setPipelineName(VkDevice device, VkPipeline pipeline, const char * name)
	{
		setObjectName(device, (uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
	}

	void setPipelineLayoutName(VkDevice device, VkPipelineLayout pipelineLayout, const char * name)
	{
		setObjectName(device, (uint64_t)pipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
	}

	void setRenderPassName(VkDevice device, VkRenderPass renderPass, const char * name)
	{
		setObjectName(device, (uint64_t)renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
	}

	void setFramebufferName(VkDevice device, VkFramebuffer framebuffer, const char * name)
	{
		setObjectName(device, (uint64_t)framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
	}

	void setDescriptorSetLayoutName(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const char * name)
	{
		setObjectName(device, (uint64_t)descriptorSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
	}

	void setDescriptorSetName(VkDevice device, VkDescriptorSet descriptorSet, const char * name)
	{
		setObjectName(device, (uint64_t)descriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name);
	}

	void setSemaphoreName(VkDevice device, VkSemaphore semaphore, const char * name)
	{
		setObjectName(device, (uint64_t)semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
	}

	void setFenceName(VkDevice device, VkFence fence, const char * name)
	{
		setObjectName(device, (uint64_t)fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
	}

	void setEventName(VkDevice device, VkEvent _event, const char * name)
	{
		setObjectName(device, (uint64_t)_event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name);
	}
};
K3D_VK_END