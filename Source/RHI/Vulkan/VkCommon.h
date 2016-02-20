#ifndef __VkCommon_h__
#define __VkCommon_h__
#pragma once

#define K3D_VK_BEGIN namespace k3d { namespace vk {
#define K3D_VK_END }}

#include <Kaleido3D.h>
#include <vulkan/vulkan.h>
#include <RHI/IRHI.h>
#include <string>
#include <vector>
#include <array>
#include <Core/LogUtil.h>


#define K3D_VK_VERIFY(expr) \
	do { \
		if ((expr) != VK_SUCCESS) { \
		Log::Out (LogLevel::Fatal, "VKRHI_ASSERT", "failed "## #expr ##" %s@%d.", __FILE__, __LINE__); \
		throw; \
		}\
	} while (0);

#endif