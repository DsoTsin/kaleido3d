include_directories(
../.. ../../../Include
${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12
)

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

set_target_properties(Render PROPERTIES FOLDER "Runtime")