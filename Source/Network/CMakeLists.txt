include_directories(.. ../../Include)

#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

set(SOCKET_SRCS
	IPv4Address.h
	IPv4Address.cpp
	Socket.h
	Socket.cpp
	TCPClient.h
	TCPClient.cpp
	NetConnection.h
	NetConnection.cpp
)

source_group(Socket FILES
	${SOCKET_SRCS}
)

add_library(Network  ${LIB_TYPE} 
	${SOCKET_SRCS}
)

add_precompiled_header(Network "Kaleido3D.h" "${Kaleido3D_ROOT_DIR}/Include" "${Kaleido3D_ROOT_DIR}/Source/Network/Socket.cpp")
set_target_properties(Network PROPERTIES FOLDER "Runtime")