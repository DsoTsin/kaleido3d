message("ThirdParty includes vulkan, rapidjson, glslang, spirv2cross, freetype2. ")
message("build config = ${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}")

if(WIN32)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Win64_${CMAKE_BUILD_TYPE})
elseif(ANDROID)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Android_ARM_${CMAKE_BUILD_TYPE})
elseif(MACOS)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/MacOS_${CMAKE_BUILD_TYPE})
endif()

unset(THIRDPARTY_FOUND CACHE)

unset(VULKANSDK_INCLUDE_DIR CACHE)
unset(RAPIDJSON_INCLUDE_DIR CACHE)
unset(GLSLANG_INCLUDE_DIR CACHE)
unset(SPIRV2CROSS_INCLUDE_DIR CACHE)
unset(FREETYPE2_INCLUDE_DIR CACHE)

set(FREETYPE2_LIBRARY freetyped)
set(GLSLANG_LIBRARIES glslang HLSL OGLCompiler OSDependent SPIRV SPVRemapper)
set(SPIRV2CROSS_LIBRARY Spirv2Cross)

if(WIN32)
    unset(DXSDK_INCLUDE_DIR CACHE)
endif()

find_path(VULKANSDK_INCLUDE_DIR
    vulkan/vulkan.h
    PATH_SUFFIXES include
    PATHS ${ThirdParty_PREBUILT_DIR}
)

find_path(RAPIDJSON_INCLUDE_DIR
    rapidjson/rapidjson.h
    PATH_SUFFIXES include
    PATHS ${ThirdParty_PREBUILT_DIR}
)

find_path(GLSLANG_INCLUDE_DIR
    glslang/GlslangToSpv.h
    PATH_SUFFIXES include
    PATHS ${ThirdParty_PREBUILT_DIR}
)

find_path(SPIRV2CROSS_INCLUDE_DIR
    spirv2cross/spirv.hpp
    PATH_SUFFIXES include
    PATHS ${ThirdParty_PREBUILT_DIR}
)

find_path(FREETYPE2_INCLUDE_DIR
    freetype2/ft2build.h
    PATH_SUFFIXES include
    PATHS ${ThirdParty_PREBUILT_DIR}
)

message("GLSLang = ${GLSLANG_INCLUDE_DIR}")

if(WIN32)
	find_path(DXSDK_INCLUDE_DIR
	    d3d12.h
	    PATH_SUFFIXES include
	    PATHS ${ThirdParty_PREBUILT_DIR}
	)
endif()

mark_as_advanced(VULKANSDK_INCLUDE_DIR)
mark_as_advanced(RAPIDJSON_INCLUDE_DIR)
mark_as_advanced(GLSLANG_INCLUDE_DIR)
mark_as_advanced(SPIRV2CROSS_INCLUDE_DIR)
mark_as_advanced(FREETYPE2_INCLUDE_DIR)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

	#find_library(LUA_LIBRARY lua51.lib
    #	HINTS ${_LUA_LIBRARY_DIR}
    #	PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)

endif()

include_directories(${ThirdParty_PREBUILT_DIR}/include)
link_directories(${ThirdParty_PREBUILT_DIR}/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ThirdParty DEFAULT_MSG
    VULKANSDK_INCLUDE_DIR
    RAPIDJSON_INCLUDE_DIR
    GLSLANG_INCLUDE_DIR
    SPIRV2CROSS_INCLUDE_DIR
    FREETYPE2_INCLUDE_DIR
)

mark_as_advanced(
    VULKANSDK_INCLUDE_DIR
    RAPIDJSON_INCLUDE_DIR
    GLSLANG_INCLUDE_DIR
    SPIRV2CROSS_INCLUDE_DIR
    FREETYPE2_INCLUDE_DIR
)