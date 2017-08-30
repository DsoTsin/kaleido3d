set(MODULE_NAME Network)
include_directories(.. ../../Include . Public)

if(BUILD_WITH_OPENSSL)
    add_definitions(-DWITH_OPENSSL=1)
endif()

set(Http_SRCS 
    Public/HttpModule.h 
    Public/Network.h 
    Private/HttpModule.cpp
    Private/Network.cpp)
source_group(Http FILES ${Http_SRCS})
add_plugin(${MODULE_NAME} FOLDER "Runtime/Plugins" SRCS ${Http_SRCS} LIBS ${OPENSSL_LIBRARIES})
add_precompiled_header(${MODULE_NAME} "Network.h" "Public" "Private/Network.cpp")


if(BUILD_WITH_UNIT_TEST)
    add_subdirectory(UnitTest)
endif()