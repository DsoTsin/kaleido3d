file(GLOB VKRHI_SRC_LIST "*.cpp" "*.h")
include_directories(
../.. ../../../Include
.
)

if(WIN32)
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()

#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

set(RHI_HEADERS
	../IRHI.h
	Public/VkRHI.h
	VkCommon.h
	Public/VkObjects.h
	Public/VkObjects.cpp
	Public/DynVulkanLoader.h
)

set(RHI_IMPL_SRCS
	Private/VkDevice.cpp
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
	private/VkRenderPass.cpp
	Private/VkDebug.cpp
	Public/VkRHI.cpp
	Public/DynVulkanLoader.cpp
)

source_group(
	Public FILES
	${RHI_HEADERS}
)

source_group(
	Private FILES
	${RHI_IMPL_SRCS}
)

add_library(RHI_Vulkan  ${LIB_TYPE} 
	${VKRHI_SRC_LIST} 
	${RHI_HEADERS}
	${RHI_IMPL_SRCS}
)

target_link_libraries(
	RHI_Vulkan
	Core Engine ShaderCompiler
)

add_precompiled_header(RHI_Vulkan "VkCommon.h" "${Kaleido3D_ROOT_DIR}/Source/RHI/Vulkan" "${Kaleido3D_ROOT_DIR}/Source/RHI/Vulkan/Public/VkRHI.cpp")
set_target_properties(RHI_Vulkan PROPERTIES FOLDER "RHI")

install(TARGETS RHI_Vulkan ARCHIVE DESTINATION lib)