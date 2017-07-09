generate_win_res(vulkanc VER "1.0.0.1" DESC "Next Generation Graphics Shader Compiler - Vulkan")
add_executable(vulkanc vulkanc.cpp ${vulkanc_RC})
target_link_libraries(vulkanc ngfx_vulkan_glslang)
set_target_properties(vulkanc PROPERTIES FOLDER "ngfx/tools")