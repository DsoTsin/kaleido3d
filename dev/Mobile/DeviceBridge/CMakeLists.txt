
include_directories(Public)
include_directories(Private)

include(${K3D_DEPENDS_PATH}/protobuf.cmake)
set(GRPC_CPP_PLUGIN_EXECUTABLE "${K3D_DEPENDS_PATH}/bin/win64_vc150r/grpc_cpp_plugin.exe")
set(PROTOC_EXECUTABLE "${K3D_DEPENDS_PATH}/bin/win64_vc150r/protoc.exe")
function(add_proto TARGET)
    cmake_parse_arguments(${TARGET}
        ""
        "FOLDER;OUTPUT_VAR;GEN_DIR;INC_DIR"
        "PROTOS"
        ${ARGN}
    )
    foreach(PROTO IN LISTS ${TARGET}_PROTOS)
    	get_filename_component(ABS_PROTO ${PROTO} ABSOLUTE)
    	get_filename_component(PROTO_NAME ${PROTO} NAME_WE)
    	#add_custom_command(TARGET ${TARGET} PRE_BUILD COMMAND
		execute_process(COMMAND
			${PROTOC_EXECUTABLE}
			--grpc_out=generate_mock_code=true:${${TARGET}_GEN_DIR}
			--cpp_out=${${TARGET}_GEN_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
			-I ${${TARGET}_INC_DIR}
			${ABS_PROTO}
        )
        
        list(APPEND PROTOSRC_LIST ${${TARGET}_GEN_DIR}/${PROTO_NAME}.pb.cc ${${TARGET}_GEN_DIR}/${PROTO_NAME}.grpc.pb.cc)
    endforeach()
    include_directories("${CMAKE_BINARY_DIR}/CMakeFiles")
    set(${${TARGET}_OUTPUT_VAR} ${PROTOSRC_LIST} PARENT_SCOPE)
endfunction()

set(IOS_BRIDGE_SRCS 
Private/iOS/Bridge.h
Private/iOS/Bridge.cpp
)
source_group(iOS FILES ${IOS_BRIDGE_SRCS})

set(ANDROID_BRIDGE_SRCS 
Private/Android/Bridge.h
Private/Android/Bridge.cpp
Private/Android/AdbHelper.h
Private/Android/AdbHelper.cpp
Private/Android/DeviceMonitor.h
Private/Android/DeviceMonitor.cpp
Private/Android/Jdwp.h
Private/Android/Jdwp.cpp
)
source_group(Android FILES ${ANDROID_BRIDGE_SRCS})

set(SRV_SRC
Public/MobileDeviceBridge.h
Private/Service.h Private/Service.cpp
Private/DeviceBridge.cpp
)
source_group(Service FILES ${SRV_SRC})

add_proto(DeviceBridge PROTOS
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/agent_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/cpu.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/gpu.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/event.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/graphics.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/internal_event.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/internal_memory.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/internal_network.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/io.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/memory.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/network.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/profiler.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/profiler_service.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto/simplerperf_report.proto
  OUTPUT_VAR PROTO_SRCS
  INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Private/Android/Proto
  GEN_DIR "${CMAKE_BINARY_DIR}/CMakeFiles"
)
source_group("Android\\Proto" FILES ${PROTO_SRCS})

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

add_library(DeviceBridge SHARED ${IOS_BRIDGE_SRCS} ${ANDROID_BRIDGE_SRCS} ${SRV_SRC} ${PROTO_SRCS})
add_lib_api(DeviceBridge SHARED)
target_link_libraries(DeviceBridge Kaleido3D.Core imobiledevice protobuf grpc gpr grpc++ ssl crypto zlib ws2_32)
set_target_properties(DeviceBridge PROPERTIES FOLDER "Tools/Mobile")