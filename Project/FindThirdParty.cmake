message("ThirdParty includes vulkan, rapidjson, glslang, spirv2cross, freetype2. ")
set(GIT_THIRDPARTY_REPO "https://github.com/Tomicyo/kaleido3d_dep_prebuilt")
if(WIN32)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
	   set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Win64/${CMAKE_BUILD_TYPE})
    else()
       set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Win32/${CMAKE_BUILD_TYPE})
    endif()
    if(NOT (EXISTS ${ThirdParty_PREBUILT_DIR}))
        message(AUTHOR_WARNING "cloning ${ThirdParty_PREBUILT_DIR} from ${GIT_THIRDPARTY_REPO}")
        execute_process(COMMAND git clone ${GIT_THIRDPARTY_REPO} -b win64_${CMAKE_BUILD_TYPE} ${ThirdParty_PREBUILT_DIR})
    endif()
elseif(ANDROID)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/Android/${ANDROID_ABI}/${ANDROID_STL}/${CMAKE_BUILD_TYPE})
    if(NOT (EXISTS ${ThirdParty_PREBUILT_DIR}))
        message(AUTHOR_WARNING "cloning ${ThirdParty_PREBUILT_DIR} from ${GIT_THIRDPARTY_REPO}")
        execute_process(COMMAND git clone ${GIT_THIRDPARTY_REPO} -b android_${ANDROID_ABI}_${ANDROID_STL}_${CMAKE_BUILD_TYPE} ${ThirdParty_PREBUILT_DIR})
    endif()
elseif(MACOS)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/MacOS/${CMAKE_BUILD_TYPE})
    if(NOT (EXISTS ${ThirdParty_PREBUILT_DIR}))
        message(AUTHOR_WARNING "cloning ${ThirdParty_PREBUILT_DIR} from ${GIT_THIRDPARTY_REPO}")
        execute_process(COMMAND git clone ${GIT_THIRDPARTY_REPO} -b macos_${CMAKE_BUILD_TYPE} ${ThirdParty_PREBUILT_DIR})
    endif()
elseif(IOS)
	set(ThirdParty_PREBUILT_DIR ${K3D_THIRD_PARTY}/iOS/${CMAKE_BUILD_TYPE})
    if(NOT (EXISTS ${ThirdParty_PREBUILT_DIR}))
        message(AUTHOR_WARNING "cloning ${ThirdParty_PREBUILT_DIR} from ${GIT_THIRDPARTY_REPO}")
        execute_process(COMMAND git clone ${GIT_THIRDPARTY_REPO} -b ios_${CMAKE_BUILD_TYPE} ${ThirdParty_PREBUILT_DIR})
    endif()
endif()
message(STATUS "** 3rd party ** ${ThirdParty_PREBUILT_DIR}")

unset(THIRDPARTY_FOUND CACHE)

unset(VULKANSDK_INCLUDE_DIR CACHE)
unset(RAPIDJSON_INCLUDE_DIR CACHE)
unset(GLSLANG_INCLUDE_DIR CACHE)
unset(SPIRV2CROSS_INCLUDE_DIR CACHE)
unset(FREETYPE2_INCLUDE_DIR CACHE)
unset(STEAMSDK_INCLUDE_DIR CACHE)

set(FREETYPE2_LIBRARY freetype)
set(GLSLANG_LIBRARIES glslang HLSL OGLCompiler OSDependent SPIRV SPVRemapper)
set(SPIRV2CROSS_LIBRARY spirv-cross-core spirv-cross-msl spirv-cross-glsl)
set(STEAMSDK_LIBRARY steam_api64)

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

if(WIN32)
find_library(VULKAN_LIB vulkan-1
	PATH_SUFFIXES lib
	PATHS ${ThirdParty_PREBUILT_DIR})
message(STATUS "** Vulkan \(Windows\) ** ${VULKAN_LIB}")
elseif(ANDROID)
find_library(VULKAN_LIB vulkan
	PATH_SUFFIXES platforms/android-24/arch-${ANDROID_SYSROOT_ABI}/usr/lib
	PATHS $ENV{ANDROID_NDK})
message(STATUS "** Vulkan Lib \(Android\) ** ${VULKAN_LIB}")
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
    find_path(STEAMSDK_INCLUDE_DIR
        steam/steam_api.h
        PATH_SUFFIES include
        PATHS ${ThirdParty_PREBUILT_DIR}
    )
    message("STEAMSDK = ${STEAMSDK_INCLUDE_DIR}")
    find_library(STEAMSDK_LIBRARY
        NAMES steam_api64 steam_api
        PATH_SUFFIXES lib
        PATHS
        ${ThirdParty_PREBUILT_DIR}
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
    STEAMSDK_INCLUDE_DIR
)
mark_as_advanced(
    VULKANSDK_INCLUDE_DIR
    RAPIDJSON_INCLUDE_DIR
    GLSLANG_INCLUDE_DIR
    SPIRV2CROSS_INCLUDE_DIR
    FREETYPE2_INCLUDE_DIR
    STEAMSDK_INCLUDE_DIR
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