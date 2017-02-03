#include "VkCommon.h"
#include "VkConfig.h"
#include "VkObjects.h"

K3D_VK_BEGIN

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
	static char msg[4096] = { 0 };
	switch (flags)
	{
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		VKLOG(Error, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
		break;
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		VKLOG(Info, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
		break;
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		VKLOG(Debug, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
		break;
	default:
		VKLOG(Default, "[%s]\t%s\t[location] %d", pLayerPrefix, pMessage, location);
		break;
	}	
	return VK_TRUE;
}

#define __VK_GLOBAL_PROC_GET__(name, functor) fp##name = reinterpret_cast<PFN_vk##name>(functor("vk" K3D_STRINGIFY(name)))

void Instance::SetupDebugging(VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callBack)
{
	if (!m_Instance)
	{
		VKLOG(Error, "SetupDebugging Failed. (m_Instance == null)");
		return;
	}
	__VK_GLOBAL_PROC_GET__(CreateDebugReportCallbackEXT, m_VulkanLib->ResolveEntry);
	if (!fpCreateDebugReportCallbackEXT)
		GET_INSTANCE_PROC_ADDR(m_Instance, CreateDebugReportCallbackEXT);
	__VK_GLOBAL_PROC_GET__(DestroyDebugReportCallbackEXT, m_VulkanLib->ResolveEntry);
	if (!fpDestroyDebugReportCallbackEXT)
		GET_INSTANCE_PROC_ADDR(m_Instance, DestroyDebugReportCallbackEXT);
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pNext = NULL;
	dbgCreateInfo.pfnCallback = callBack;
	dbgCreateInfo.pUserData = NULL;
	dbgCreateInfo.flags = flags;
	K3D_VK_VERIFY(fpCreateDebugReportCallbackEXT(m_Instance, &dbgCreateInfo, NULL, &m_DebugMsgCallback));
}

void Instance::FreeDebugCallback()
{
	if (m_Instance && m_DebugMsgCallback)
	{
		fpDestroyDebugReportCallbackEXT(m_Instance, m_DebugMsgCallback, nullptr);
		m_DebugMsgCallback = VK_NULL_HANDLE;
	}
}

#ifdef DEBUG_MARKER
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
#endif
K3D_VK_END