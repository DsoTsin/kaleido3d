message("ThirdParty includes vulkan, rapidjson, glslang, spirv2cross, freetype2. ")
if(WIN32)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Win64_${CMAKE_BUILD_TYPE})
elseif(ANDROID)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Android_ARM_${CMAKE_BUILD_TYPE})
elseif(MACOS)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/MacOS_${CMAKE_BUILD_TYPE})
elseif(IOS)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/iOS_${CMAKE_BUILD_TYPE})
endif()
message(STATUS "** 3rd party ** ${ThirdParty_PREBUILT_DIR}")

unset(THIRDPARTY_FOUND CACHE)

unset(VULKANSDK_INCLUDE_DIR CACHE)
unset(RAPIDJSON_INCLUDE_DIR CACHE)
unset(GLSLANG_INCLUDE_DIR CACHE)
unset(SPIRV2CROSS_INCLUDE_DIR CACHE)
unset(FREETYPE2_INCLUDE_DIR CACHE)

set(FREETYPE2_LIBRARY freetype)
set(GLSLANG_LIBRARIES glslang HLSL OGLCompiler OSDependent SPIRV SPVRemapper)
set(SPIRV2CROSS_LIBRARY Spirv2Cross)

if(WIN32)
    unset(DXSDK_INCLUDE_DIR CACHE)
endif()

if(WIN32 OR ANDROID)
find_path(VULKANSDK_INCLUDE_DIR
    vulkan/vulkan.h
    PATH_SUFFIXES include
    PATHS ${ThirdParty_PREBUILT_DIR}
)
endif()

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
    ft2build.h
    PATH_SUFFIXES include/freetype2
    PATHS ${ThirdParty_PREBUILT_DIR}
)

find_library(FREETYPE2_LIBRARY
	NAMES freetype
	HINTS
	$ENV{FREETYPE2_DIR}
	PATH_SUFFIXES lib
	PATHS
	${ThirdParty_PREBUILT_DIR}
)

if(FREETYPE2_LIBRARY)
# Find dependencies
    foreach (d ZLIB BZip2 PNG HarfBuzz)
        string(TOUPPER "${d}" D)

        if (DEFINED WITH_${d} OR DEFINED WITH_${D})
            if (WITH_${d} OR WITH_${D})
            find_package(${d} QUIET REQUIRED)
            endif ()
        else ()
            find_package(${d} QUIET)
        endif ()

        if (${d}_FOUND OR ${D}_FOUND)
            message(STATUS "link FreeType with ${d}")
        endif ()
    endforeach ()

	include_directories(${FREETYPE2_INCLUDE_DIR})
	set(HAS_FREETYPE true)

    if (ZLIB_FOUND)
        list(APPEND FREETYPE2_LIBRARY ${ZLIB_LIBRARIES})
    endif ()
    if (BZIP2_FOUND)
        list(APPEND FREETYPE2_LIBRARY ${BZIP2_LIBRARIES})
    endif ()
    if (PNG_FOUND)
        list(APPEND FREETYPE2_LIBRARY ${PNG_LIBRARIES})
    endif ()
    if (HARFBUZZ_FOUND)
        list(APPEND FREETYPE2_LIBRARY ${HARFBUZZ_LIBRARIES})
    endif ()
endif()

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

if(WIN32 OR ANDROID)
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
elseif(IOS OR MACOS)
find_package_handle_standard_args(ThirdParty DEFAULT_MSG
    RAPIDJSON_INCLUDE_DIR
    GLSLANG_INCLUDE_DIR
    SPIRV2CROSS_INCLUDE_DIR
    FREETYPE2_INCLUDE_DIR
)
mark_as_advanced(
    RAPIDJSON_INCLUDE_DIR
    GLSLANG_INCLUDE_DIR
    SPIRV2CROSS_INCLUDE_DIR
    FREETYPE2_INCLUDE_DIR
)
endif()