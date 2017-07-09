find_path(V8_INCLUDE_DIR
    v8.h
    PATH_SUFFIXES include
    PATHS 
    ${V8_DIR} 
    ${K3D_THIRD_PARTY}/v8-5.5.1
)

mark_as_advanced(V8_INCLUDE_DIR)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

	set (_V8_LIBRARY_DIR  ${V8_INCLUDE_DIR}/../vs2015/debug_x64)
	set(V8_LIBRARIES icui18n.dll.lib icuuc.dll.lib libbase.lib libplatform.lib v8.dll.lib ws2_32.lib winmm.lib shlwapi.lib dbghelp.lib)

	find_library(V8_LIBRARY v8.dll.lib
    	HINTS ${_V8_LIBRARY_DIR}
    	PATHS ENV LIBRARY_PATH ENV LD_LIBRARY_PATH)

    set(V8_LIBRARY_DIR ${_V8_LIBRARY_DIR})

endif()


if (V8_LIBRARY)
    
    set (V8_FOUND "YES")

    include_directories(${V8_INCLUDE_DIR})
    link_directories(${V8_LIBRARY_DIR})

endif(V8_LIBRARY)

if (V8_FOUND)

	set(V8_VERSION 0)
	
	file(READ "${V8_INCLUDE_DIR}/v8-version.h" _V8_VERSION_CONTENTS)
	
	string(REGEX REPLACE ".*#define V8_MAJOR_VERSION ([0-9]+).*" "\\1" V8_MAJOR_VERSION "${_V8_VERSION_CONTENTS}")

	set(V8_MAJOR_VERSION "${V8_MAJOR_VERSION}")

	string(REGEX REPLACE ".*#define V8_MINOR_VERSION ([0-9]+).*" "\\1" V8_MINOR_VERSION "${_V8_VERSION_CONTENTS}")

	set(V8_MINOR_VERSION "${V8_MINOR_VERSION}")

	string(REGEX REPLACE ".*#define V8_BUILD_NUMBER ([0-9]+).*" "\\1" V8_BUILD_NUMBER "${_V8_VERSION_CONTENTS}")

	set(V8_BUILD_NUMBER "${V8_BUILD_NUMBER}")

	set(V8_VERSION "${V8_MAJOR_VERSION}.${V8_MINOR_VERSION}.${V8_BUILD_NUMBER}")

	message("-- V8_VERSION = ${V8_VERSION}")

endif (V8_FOUND)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(V8 DEFAULT_MSG
    V8_INCLUDE_DIR
    V8_LIBRARY_DIR
    V8_LIBRARIES
)

mark_as_advanced(
    V8_INCLUDE_DIR
    V8_LIBRARY_DIR
    V8_LIBRARIES
)