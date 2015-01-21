file(GLOB_RECURSE DX_RENDERER_SRC_LIST "*.cpp" "*.h")
include_directories(../.. ../../../Include)

# add_definitions(-DWIN10_PREVIEWBUILD)

source_group(
	Renderer FILES
	DirectXRenderer.h
	DirectXRenderer.cpp
	DXDevice.cpp
	DXPipeLine.h
	DXPipeLine.cpp
)

source_group(
	Shader FILES
	DXShaderDefs.h
	DXShader.h
	DXShader.cpp
	DXShaderManager.h
	DXShaderManager.cpp
)

source_group(
	Mesh FILES
	DXRenderMesh.h
	DXRenderMesh.cpp
)

source_group(
	Utils FILES
	DDSUtil/DDS.h
	DXUtil.h
	DXUtil.cpp
)

source_group(
	Common FILES
	DXCommon.h
)

source_group(
	Parameter FILES
	DXRenderParameters.h
	DXRenderParameters.cpp
)

add_library(Renderer_DX STATIC ${DX_RENDERER_SRC_LIST})
add_precompiled_header(Renderer_DX "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Renderer/DirectX/DXRenderMesh.cpp")
set_target_properties(Renderer_DX PROPERTIES FOLDER "Runtime")