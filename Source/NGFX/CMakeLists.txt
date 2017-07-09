set(NGFX_HEADER ${Kaleido3D_SOURCE_DIR}/Source/NGFX/ngfx.h)
set(NGFX_YML ${Kaleido3D_SOURCE_DIR}/Tools/RHIGenerator/ngfx.yml)

set_source_files_properties(${NGFX_HEADER}
	PROPERTIES GENERATED TRUE)

add_library(ngfx SHARED 
${NGFX_HEADER}
${NGFX_YML}
vulkan.cpp
)

add_custom_command(TARGET ngfx PRE_BUILD COMMAND
	${PYTHON_EXECUTABLE} ${Kaleido3D_SOURCE_DIR}/Tools/RHIGenerator/run.py -o 
	${NGFX_HEADER} ${NGFX_YML})

target_link_libraries(ngfx Core ${VULKAN_LIB})