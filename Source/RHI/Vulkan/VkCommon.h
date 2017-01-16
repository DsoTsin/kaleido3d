#ifndef __VkCommon_h__
#define __VkCommon_h__
#pragma once

#include <Kaleido3D.h>
#include <Math/kMath.hpp>
#include <RHI/Vulkan/Private/DynVulkanLoader.h>
#include <Interface/IRHI.h>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <Core/LogUtil.h>

#define K3D_VK_BEGIN K3D_COMMON_NS { namespace vk {
#define K3D_VK_END }}

// custom vk log callback
typedef void (*PFN_vklogCallBack)(k3d::ELogLevel const&, const char * tag, const char * fmt, ...);

extern void VkLog(k3d::ELogLevel const&, const char * tag, const char * fmt, ...);
extern void SetVkLogCallback(PFN_vklogCallBack func);

#define VKLOG(level, ...) VkLog(::k3d::ELogLevel::level, "kaleido3d::VulkanRHI", __VA_ARGS__)

#define VKRHI_METHOD_TRACE VKLOG(Info, __K3D_FUNC__);

#define K3D_VK_VERIFY(expr) \
	do { \
		if ((expr) != VK_SUCCESS) { \
		VkLog (ELogLevel::Fatal, "VKRHI_ASSERT", "failed " K3D_STRINGIFY(expr) " %s@%d.", __FILE__, __LINE__); \
		throw; \
		}\
	} while (0);

#endif