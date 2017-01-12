file(GLOB VKRHI_SRC_LIST "*.cpp" "*.h")
include_directories(
../.. ../../../Include
.
)

if(WIN32)
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
	add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
endif()

set(RHI_HEADERS
	../IRHI.h
	VkCommon.h
	Public/IVkRHI.h
)

set(RHI_IMPL_SRCS
	Private/VkRHI.h
	Private/VkObjects.h
	Private/VkObjects.cpp
	Private/DynVulkanLoader.h
	Private/VkDevice.cpp
	Private/VkEnums.h
	Private/VkUtils.h
	Private/VkUtils.cpp
	Private/VkRenderViewport.cpp
	Private/VkCommandContext.cpp
	Private/VkResource.cpp
	Private/VkPipelineState.cpp
	Private/VkSampler.cpp
	Private/VkPipelineLayout.cpp
	Private/VkDescriptor.cpp
	Private/VkSwapChain.cpp
	Private/VkFrameBuffer.cpp
	Private/VkRenderPass.cpp
	Private/VkDebug.cpp
	Private/VkImpl.cpp
	Private/VkRHI.cpp
	Private/DynVulkanLoader.cpp
)

source_group(
	Public FILES
	${RHI_HEADERS}
)

source_group(
	Private FILES
	${RHI_IMPL_SRCS}
)

add_plugin(RHI_Vulkan "Runtime/RHI" ${VKRHI_SRC_LIST} ${RHI_HEADERS} ${RHI_IMPL_SRCS} LINKLIBS ShaderCompiler)

if(NOT ANDROID)
	install(TARGETS RHI_Vulkan ARCHIVE DESTINATION lib)
	add_precompiled_header(RHI_Vulkan "VkCommon.h" "${Kaleido3D_ROOT_DIR}/Source/RHI/Vulkan" "${Kaleido3D_ROOT_DIR}/Source/RHI/Vulkan/Private/VkRHI.cpp")
endif()

