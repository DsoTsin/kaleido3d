include_directories( . )

set (MTL_SRCS
	MetalDevice.h
	MetalDevice.mm
)

source_group(
    Renderer FILES
    ${MTL_SRCS}
)

set (UTIL_SRCS
	Util/ImageUtil.h
	Util/ImageUtil.mm
)

source_group (
	Util FILES
	${UTIL_SRCS}
)

set(CMAKE_CXX_FLAGS "-fobjc-abi-version=2 -fobjc-arc ${CMAKE_CXX_FLAGS}")

add_library (Renderer_Metal STATIC
	${MTL_SRCS}
	${UTIL_SRCS}
)

set_xcode_property(Renderer_Metal GCC_GENERATE_DEBUGGING_SYMBOLS YES)

set_target_properties(Renderer_Metal PROPERTIES FOLDER "Runtime")


# Installation
# set (CMAKE_INSTALL_PREFIX "")
# install (TARGETS Renderer_Metal DESTINATION .)
# install (FILES HelloIOS.h DESTINATION .)