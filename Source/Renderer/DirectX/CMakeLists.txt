file(GLOB_RECURSE DX_RENDERER_SRC_LIST "*.cpp" "*.h")
include_directories(../.. ../../../Include)

source_group(
	Renderer FILES
	DirectXRenderer.h
	DirectXRenderer.cpp
	DXDevice.cpp
)

source_group(
	Shader FILES
	DXShader.h
	DXShader.cpp
	DXShaderManager.h
	DXShaderManager.cpp
)

source_group(
	DDSUtil FILES
	DDSUtil/DDS.h
	DDSUtil/DDSTextureLoader.h
	DDSUtil/DDSTextureLoader.cpp
)

source_group(
	Mesh FILES
	DXRenderMesh.h
	DXRenderMesh.cpp
)

add_library(Renderer_DX STATIC ${DX_RENDERER_SRC_LIST})