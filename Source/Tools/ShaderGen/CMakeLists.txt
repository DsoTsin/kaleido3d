
#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

add_library(
ShaderCompiler ${LIB_TYPE} 
ShaderCompiler.cc
DXCompiler.cc
GLSLCompiler.cc
)

target_link_libraries(
ShaderCompiler
glslang
OGLCompiler
OSDependent
SPIRV
HLSL
)

add_executable(
ShaderGen
ShaderGen.cc
)

target_link_libraries(
ShaderGen
ShaderCompiler
)


set_target_properties(ShaderGen PROPERTIES FOLDER "Tools")
set_target_properties(ShaderCompiler PROPERTIES FOLDER "Tools")