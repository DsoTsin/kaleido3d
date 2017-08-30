include_directories(.. ../../Include)

set(SCRIPT_SRCS
V8Binding.h
V8Binding.cpp
)

source_group(V8 FILES
	${SCRIPT_SRCS}
)

add_library(V8Script ${LIB_TYPE}
	${SCRIPT_SRCS}
)

target_link_libraries(
V8Script
Kaleido3D.Core
RHI_Vulkan
ngfx_vulkan
${V8_LIBRARIES}
)

#add_precompiled_header(V8Script "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Script/V8Binding.cpp")
set_target_properties(V8Script PROPERTIES FOLDER "Runtime/Plugins")
