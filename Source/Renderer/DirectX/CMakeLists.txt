file(GLOB_RECURSE DX_RENDERER_SRC_LIST "*.cpp" "*.h")
include_directories(../.. ../../../Include)
add_library(Renderer_DX STATIC ${DX_RENDERER_SRC_LIST})