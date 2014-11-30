include_directories(
../.. ../../../Include
)

set(RENDER_SRCS
Render.h
Render.cpp
MeshRender.cpp
)

add_library(Render ${LIB_TYPE}
${RENDER_SRCS}
)

set_target_properties(Render PROPERTIES FOLDER "Runtime")