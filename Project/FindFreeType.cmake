find_path(FREETYPE2_INCLUDE_DIR ft2build.h
	HINTS
	$ENV{FREETYPE2_DIR}
	PATH_SUFFIXES include
	PATHS
	${K3D_THIRD_PARTY}/FreeType-2.7
)

find_library(FREETYPE2_LIBRARY
	NAMES freetype27
	HINTS
	$ENV{FREETYPE2_DIR}
	PATH_SUFFIXES lib objs/vc2010/x64
	PATHS
	${K3D_THIRD_PARTY}/FreeType-2.7
)

if(FREETYPE2_LIBRARY)
	include_directories(${FREETYPE2_INCLUDE_DIR})
	set(HAS_FREETYPE true)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeType REQUIRED_VARS FREETYPE2_INCLUDE_DIR FREETYPE2_LIBRARY)

