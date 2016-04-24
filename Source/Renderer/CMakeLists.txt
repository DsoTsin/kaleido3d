include_directories(
../.. ../../../Include
${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12
${DXSDK_INCLUDE_DIR}
)

#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

_add_package_VulkanSDK()

set(RENDER_SRCS
Render.h
Render.cpp
MeshRender.cpp
RenderContext.h
RenderContext.cpp
DynamicRHILoader.h
DynamicRHILoader.cpp
)

add_library(Render ${LIB_TYPE}
${RENDER_SRCS}
)

target_link_libraries(
Render
Core
RHI_Vulkan
RHI_D3D12
)

set_target_properties(Render PROPERTIES FOLDER "Runtime")