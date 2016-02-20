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

set(VKTSBASE_SRCS
Base/vulkantools.h
Base/vulkantools.cpp
Base/vulkandebug.h
Base/vulkandebug.cpp
Base/vulkanswapchain.hpp
)

source_group(
	Base FILES
	${VKTSBASE_SRCS}
)

set(RHI_HEADERS
	../IRHI.h
	Public/VkRHI.h
	VkCommon.h
)

set(RHI_IMPL_SRCS
	Private/VkImpl.cpp
	Private/VkDevice.cpp
	Private/VkUtils.h
	Private/VkUtils.cpp
	Private/VkRenderViewport.cpp
	Private/VkCommandContext.cpp
	Private/VkResource.cpp
	Private/VkShaderCompiler.cpp
	Private/VkPipelineState.cpp
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
	${VKTSBASE_SRCS}
)

target_link_libraries(
	RHI_Vulkan
	Core Engine
)

add_precompiled_header(RHI_Vulkan "VkCommon.h" "${Kaleido3D_ROOT_DIR}/Source/RHI/Vulkan" "${Kaleido3D_ROOT_DIR}/Source/RHI/Vulkan/Private/VkImpl.cpp")
set_target_properties(RHI_Vulkan PROPERTIES FOLDER "RHI")