
set(SOURCES
    metalib2spirv_p.h
    metalib2spirv.cc
    metalib.cc)

source_group(Metal FILES ${SOURCES})

set(SPV_SOURCES
    spirv_builder.h
    spirv_builder.cc
    spirv_writer.cc
    spirv_util.h
    spirv_util.cc)

if (MSVC)
    list(APPEND SPV_SOURCES metalib-spirv.natvis)
endif()

source_group(SPIRV FILES ${SPV_SOURCES})

add_library(metalib2spirv SHARED ${SOURCES} ${SPV_SOURCES})
set_target_properties(metalib2spirv PROPERTIES CXX_STANDARD 17)
target_compile_definitions(metalib2spirv PRIVATE BUILD_LIB=1)
target_include_directories(metalib2spirv PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Metal/source
    ${CMAKE_SOURCE_DIR}/src/Metal/include
    ${CMAKE_SOURCE_DIR}/build/deps/include
)

if(LLVM_FOUND)
    target_link_libraries(metalib2spirv PRIVATE LLVMCore LLVMBitReader LLVMSupport)
else()
    target_link_libraries(metalib2spirv PRIVATE llvm::core llvm::bit_reader llvm::support llvm::bitstream_reader 
    llvm::remarks llvm::binary_format
    )
endif()