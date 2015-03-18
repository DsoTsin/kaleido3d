file(GLOB_RECURSE RENDERER_SRC_LIST "*.cpp" "*.hpp" "*.h")

set(GLEW_SOURCES_DIR ${K3D_THIRD_PARTY}/glew)

include_directories(
	../..
	../../../Include
	${GLEW_SOURCES_DIR}/include
)

source_group(
	OpenGL FILES 
	${RENDERER_SRC_LIST}
)

set(
	GLEW_SRC 
	${GLEW_SOURCES_DIR}/include/GL/glew.h
	${GLEW_SOURCES_DIR}/src/glew.cpp
)

source_group(
	GLEW FILES
	${GLEW_SRC}
)

source_group(
	Nvidia FILES
	Nvidia/NVCommandList.h Nvidia/NVCommandList.cpp
	Nvidia/NVToken.h Nvidia/NVToken.cpp
)

source_group(
	Renderer FILES
	OGLRenderer.h OGLRenderer.cpp OGLDevice.cpp
)


source_group(
	DDSHelper FILES
	DDSHelper/DDSHelper.h
	DDSHelper/DDSHelper.cpp	
)

source_group(
	Shader FILES
	OGLShader.h OGLShader.cpp
)

source_group(
	Texture FILES
	OGLTexture.h OGLTexture.cpp
)

source_group(
	Buffer FILES
	OGLBuffer.h OGLBuffer.cpp
)

source_group(
	CmdList FILES
	OGLCmdList.h OGLCmdList.cpp
)

add_definitions(-DGLEW_STATIC)

add_library(
	Renderer_MOGL STATIC
	${RENDERER_SRC_LIST}
	${GLEW_SRC}
)

set_target_properties(Renderer_MOGL PROPERTIES FOLDER "Runtime")
add_precompiled_header(Renderer_MOGL "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Renderer/MOGL/CommonGL.cpp")