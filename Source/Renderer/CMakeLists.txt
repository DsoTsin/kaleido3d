include_directories(
../.. ../../../Include
${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12
${DXSDK_INCLUDE_DIR}
${V8_INCLUDE_DIR}
)

if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB -DBUILD_WITH_PLUGIN)
endif()

set(RENDER_SRCS
Render.h
Render.cpp
MeshRender.cpp
RenderContext.cpp
)

source_group(Render FILES ${RENDER_SRCS})

set(RENDER_DEPLIB Core)

if(HAS_FREETYPE)
	set(FTRENDER_SRC FontRenderer.h FontRenderer.cpp)
	source_group(Font FILES ${FTRENDER_SRC})
	set(RENDER_SRCS ${RENDER_SRCS} ${FTRENDER_SRC})
	set(RENDER_DEPLIB ${RENDER_DEPLIB} ${FREETYPE2_LIBRARY})
endif()

add_library(Render ${LIB_TYPE} ${RENDER_SRCS})
target_link_libraries(Render ${RENDER_DEPLIB})
set_target_properties(Render PROPERTIES FOLDER "Runtime")

if(WIN32)
	install(TARGETS Render ARCHIVE DESTINATION lib)
elseif(MACOS)
	if(BUILD_SHARED)
		add_custom_command(TARGET Render POST_BUILD COMMAND
			${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/libRender.dylib" $<TARGET_FILE:Render>)
	endif()
endif()
