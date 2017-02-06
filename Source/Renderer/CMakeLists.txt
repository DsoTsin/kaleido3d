include_directories(
../.. ../../../Include
${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12
${DXSDK_INCLUDE_DIR}
${V8_INCLUDE_DIR}
)

set(RENDER_SRCS
Render.h
Render.cpp
MeshRender.cpp
RenderContext.cpp
)

if(BUILD_SHARED)
	add_definitions(-DBUILD_WITH_PLUGIN)
endif()

source_group(Render FILES ${RENDER_SRCS})

set(RENDER_DEPLIB Core)

if(HAS_FREETYPE)
	set(FTRENDER_SRC FontRenderer.h FontRenderer.cpp)
	source_group(Font FILES ${FTRENDER_SRC})
	set(RENDER_SRCS ${RENDER_SRCS} ${FTRENDER_SRC})
	set(RENDER_DEPLIB ${RENDER_DEPLIB} ${FREETYPE2_LIBRARY})
endif()

k3d_add_lib(Render SRCS ${RENDER_SRCS} LIBS ${RENDER_DEPLIB} FOLDER "Runtime")

if(MACOS AND BUILD_SHARED)
	add_custom_command(TARGET Render POST_BUILD COMMAND
		${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/libRender.dylib" $<TARGET_FILE:Render>)
endif()

install(TARGETS Render 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)