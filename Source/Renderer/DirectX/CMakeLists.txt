file(GLOB DX_RENDERER_SRC_LIST "*.cpp" "*.h")
include_directories(../.. ../../../Include)

# add_definitions(-DWIN10_PREVIEWBUILD)

source_group(
	Renderer FILES
	Renderer.h Renderer.cpp
	RootSignature.h RootSignature.cpp
	PipelineState.h PipelineState.cpp
	RenderManager.h RenderManager.cpp
)

source_group(
	Device FILES
	Device.h
	Device.cpp
	DeviceManager.h
	DeviceManager.cpp
)

source_group(
	Shader FILES
	Shader.h
	ShaderManager.h ShaderManager.cpp
	ShaderCompileWorker.h ShaderCompileWorker.cpp
)

source_group(
	Mesh FILES
	RenderMesh.h
	RenderMesh.cpp
)

source_group(
	Utils FILES
	DDSUtil/DDS.h
	Helper11.cpp
	Helper.h
	Helper.cpp
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

set(INFRAS_SRCS
	Infrastructure/IndexBuffer.h
	Infrastructure/IndexBuffer.cpp
	Infrastructure/VertexBuffer.h
	Infrastructure/VertexBuffer.cpp
	Infrastructure/UAV.h
	Infrastructure/UAV.cpp
	Infrastructure/UniformBuffer.h
	Infrastructure/UniformBuffer.cpp
	Infrastructure/StructuredBuffer.h
	Infrastructure/StructuredBuffer.cpp
	Infrastructure/Texture.h
	Infrastructure/Texture.cpp
	Resource.h
	Resource.cpp
	Wrapper.h
	Wrapper.cpp
)

source_group(
	Infrastructure FILES
	${INFRAS_SRCS}
)

add_library(Renderer_DX STATIC
	${DX_RENDERER_SRC_LIST} 
	${INFRAS_SRCS}
)

add_precompiled_header(Renderer_DX "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Renderer/DirectX/RenderMesh.cpp")
set_target_properties(Renderer_DX PROPERTIES FOLDER "Runtime")