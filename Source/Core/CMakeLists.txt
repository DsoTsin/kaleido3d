file(GLOB_RECURSE CORE_SRC_LIST "*.cpp")
include_directories(.. ../../Include)
add_library(Core STATIC ${CORE_SRC_LIST})