
set(SOURCES
    ../../include/metal_compiler.h
    metal_compiler.cc)

source_group(source FILES ${SOURCES})

add_library(metal_compiler SHARED ${SOURCES})
set_target_properties(metal_compiler PROPERTIES CXX_STANDARD 17)
target_compile_definitions(metal_compiler PRIVATE BUILD_LIB=1)
#target_link_libraries(metal_compiler LLVMCore LLVMBitReader LLVMSupport)