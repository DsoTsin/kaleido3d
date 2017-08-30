add_definitions(-DUSE_GLSLANG=1 -DAMD_EXTENSIONS)
include_directories(${DXSDK_INCLUDE_DIR} .)

set(COMPILER_SRCS
    Private/ShaderCompiler.cc Private/DXCompiler.cc Private/GLSLCompiler.cc
    Private/DXCompiler.h Private/GLSLCompiler.h
    Private/GLSLangUtils.h Private/GLSLangUtils.cc
    Private/SPIRVCrossUtils.h Private/SPIRVCrossUtils.cc
    Private/MetalCompiler.cc Private/MetalCompiler.h
	../Core/Interface/ShaderCommon.h
)

source_group("Compiler\\Private" FILES ${COMPILER_SRCS})
set(PUB_SRCS Public/ShaderCompiler.h)
source_group("Compiler\\Public" FILES ${PUB_SRCS})

add_plugin(ShaderCompiler 
	FOLDER "Runtime/Tools"
	SRCS ${COMPILER_SRCS} ${PUB_SRCS}
    LIBS ${GLSLANG_LIBRARIES} ${SPIRVCROSS_LIBRARIES})

if(MACOS AND BUILD_SHARED)
    add_custom_command(TARGET ShaderCompiler POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/libShaderCompiler.dylib" $<TARGET_FILE:ShaderCompiler>)
endif()

install(TARGETS ShaderCompiler 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
    
if(BUILD_WITH_UNIT_TEST)
    add_subdirectory(UnitTest)
endif()
