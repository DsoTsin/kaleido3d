add_definitions(-DUSE_GLSLANG=1)

include_directories(${DXSDK_INCLUDE_DIR} .)
ADD_DEFINITIONS(-DAMD_EXTENSIONS)
set(COMPILER_SRCS
    Private/ShaderCompiler.cc Private/DXCompiler.cc Private/GLSLCompiler.cc
    Private/DXCompiler.h Private/GLSLCompiler.h
    Private/GLSLangUtils.h Private/GLSLangUtils.cc
    Private/SPIRVCrossUtils.h Private/SPIRVCrossUtils.cc
    Private/MetalCompiler.cc Private/MetalCompiler.h
)

source_group("Compiler\\Private" FILES ${COMPILER_SRCS})
set(PUB_SRCS Public/ShaderCompiler.h)
source_group("Compiler\\Public" FILES ${PUB_SRCS})

add_plugin(ShaderCompiler "Tools" ${COMPILER_SRCS} ${PUB_SRCS})
target_link_libraries(ShaderCompiler ${GLSLANG_LIBRARIES} ${SPIRV2CROSS_LIBRARY})

if(MACOS)
    add_custom_command(TARGET ShaderCompiler POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/libShaderCompiler.dylib" $<TARGET_FILE:ShaderCompiler>)
endif()
