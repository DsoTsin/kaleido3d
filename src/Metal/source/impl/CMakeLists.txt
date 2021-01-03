set(vk_SOURCES
    vk/common_priv.h
    vk/common.cpp
    vk/device.cpp
    vk/pipeline.cpp
    vk/commands.cpp
    vk/resource.cpp
    vk/memory.h
    vk/memory.cpp
    ../../include/metal/common.h
    ../../include/metal/commands.h
    ../../include/metal/device.h
    ../../include/metal/enums.h
    ../../include/metal/pipeline.h
    ../../include/metal/resource.h
)

source_group(vk FILES ${vk_SOURCES})

add_library(Metal.Framework SHARED ${vk_SOURCES})
target_include_directories(Metal.Framework PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/metal
    )
target_compile_definitions(Metal.Framework PRIVATE
    BUILD_SHARED_LIB=1
    )