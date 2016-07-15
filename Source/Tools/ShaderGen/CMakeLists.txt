
#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

add_definitions(-DUSE_GLSLANG=1)

include_directories(${K3D_THIRD_PARTY}/Spir2Cross)

set(HEADERS DXCompiler.h GLSLCompiler.h ShaderCompiler.h)
set(SRCS 
ShaderCompiler.cc
DXCompiler.cc
GLSLCompiler.cc
)
source_group(
	Include FILES
	${HEADERS}
)
source_group(
	Source FILES
)

add_library(
ShaderCompiler ${LIB_TYPE} 
${SRCS}
)

target_link_libraries(
ShaderCompiler
glslang
OGLCompiler
OSDependent
SPIRV
HLSL
Spir2Cross
Core
)

#add_executable(
#ShaderGen
#ShaderGen.cc
#)

#target_link_libraries(
#ShaderGen
#ShaderCompiler
#)


#set_target_properties(ShaderGen PROPERTIES FOLDER "Tools")
set_target_properties(ShaderCompiler PROPERTIES FOLDER "Tools")