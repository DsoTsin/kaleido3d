file(GLOB OGLRHI_SRC_LIST "*.cpp" "*.h")
include_directories(
.
)

if(WIN32 OR MACOS)
	add_definitions(-DUSE_DESKTOP_OPENGL)
elseif(ANDROID OR IOS)
	add_definitions(-DUSE_OPENGL_ES)
endif()

set(RHI_HEADERS
	OpenGLCommon.h
	Public/IOpenGLRHI.h
)

set(RHI_IMPL_SRCS
	Private/OpenGLRHI.cpp
)

source_group(
	Public FILES
	${RHI_HEADERS}
)

source_group(
	Private FILES
	${RHI_IMPL_SRCS}
)

add_plugin(RHI_OpenGL FOLDER "Runtime/RHI" SRCS ${OGLRHI_SRC_LIST} ${RHI_HEADERS} ${RHI_IMPL_SRCS})

if(NOT ANDROID)
	#install(TARGETS RHI_OpenGL ARCHIVE DESTINATION lib)
	add_precompiled_header(RHI_OpenGL "OpenGLCommon.h" "${Kaleido3D_ROOT_DIR}/Source/RHI/OpenGL" "${Kaleido3D_ROOT_DIR}/Source/RHI/OpenGL/Private/OpenGLRHI.cpp")
endif()

