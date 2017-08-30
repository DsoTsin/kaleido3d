include_directories(
${Kaleido3D_ROOT_DIR}/Source
${Kaleido3D_ROOT_DIR}/Include
${Kaleido3D_ROOT_DIR}/Source/Renderer
${Kaleido3D_ROOT_DIR}/Source/Renderer/Public
)

set(RENDER_SRCS
Public/Renderer.h
Private/Render.cpp
Private/MeshRender.cpp
Private/RenderContext.cpp
)

if(BUILD_SHARED)
	add_definitions(-DBUILD_WITH_PLUGIN)
endif()

source_group(Render FILES ${RENDER_SRCS})

set(RENDER_DEPLIB Kaleido3D.Core)

if(HAS_FREETYPE)
	set(FTRENDER_SRC Private/FontRenderer.h Private/FontRenderer.cpp)
	source_group(Font FILES ${FTRENDER_SRC})
	set(RENDER_SRCS ${RENDER_SRCS} ${FTRENDER_SRC})
	set(RENDER_DEPLIB ${RENDER_DEPLIB} ${FREETYPE_LIBRARIES})
endif()

k3d_add_lib(Renderer 
    SRCS ${RENDER_SRCS} 
    LIBS ${RENDER_DEPLIB} 
    FOLDER "Runtime"
    PCH "Renderer.h"
    PCH_SRC "${Kaleido3D_ROOT_DIR}/Source/Renderer/Private/RenderContext.cpp"
    PCH_INCLUDE "${Kaleido3D_ROOT_DIR}/Source/Renderer/Public"
)

install(TARGETS Kaleido3D.Renderer
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)