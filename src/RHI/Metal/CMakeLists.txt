include_directories(.)
set(TARGET RHI_Metal)

set(SRCS 
Private/MetalRHI.mm
Private/MetalRHI.h
)

set(INCS
Public/IMetalRHI.h
)

source_group(Private FILES ${SRCS})
source_group(Public FILES ${INCS})
set(CMAKE_CXX_FLAGS "-fobjc-abi-version=2 -fobjc-arc ${CMAKE_CXX_FLAGS}")

#add_plugin(${TARGET}
#    FOLDER "Runtime/RHI"
#    SRCS ${SRCS} ${INCS})

#if(MACOS)
#target_link_libraries(${TARGET} "-framework Metal" "-framework MetalKit" "-framework AppKit" "-framework Foundation")
#elseif(IOS)
#target_link_libraries(${TARGET} "-framework Metal" "-framework MetalKit" "-framework UIKit" "-framework Foundation")
#endif()
