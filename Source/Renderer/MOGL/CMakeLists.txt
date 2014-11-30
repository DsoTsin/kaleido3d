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
	${GLEW_SOURCES_DIR}/src/glew.c
)

source_group(
	GLEW FILES
	${GLEW_SRC}
)

source_group(
	Nvidia FILES
	Nvidia/NVCommandList.h Nvidia/NVCommandList.cpp
)

source_group(
	GLDevice FILES
	OGLDevice.hpp OGLDevice.cpp
)

source_group(
	Renderer FILES
	OGLRenderer.h OGLRenderer.cpp
)

add_definitions(-DGLEW_STATIC)

add_library(
	Renderer_MOGL STATIC
	${RENDERER_SRC_LIST}
	${GLEW_SRC}
)