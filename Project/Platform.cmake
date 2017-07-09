if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "-fno-objc-arc -x objective-c++ ${CMAKE_CXX_FLAGS}")
    if(IOS)
        add_definitions(-DK3DPLATFORM_OS_IOS=1)
        set(K3D_TARGET_SYSTEM "iOS")
    else()
        set(MACOS TRUE)
        add_definitions(-DK3DPLATFORM_OS_MAC=1)
        set(K3D_TARGET_SYSTEM "MacOS")
		set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
		set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.tsinstudio.kaleido3d")
		set(MACOSX_BUNDLE_COPYRIGHT "Copyright Tsin Studio 2016. All Rights Reserved.")
    endif()
endif()

if(WIN32)
    set(K3D_TARGET_SYSTEM "Windows")
endif(WIN32)

if(ANDROID) # Android will above vars
    set(K3D_TARGET_SYSTEM "Android")
endif(ANDROID)

message(STATUS "Host System = ${CMAKE_SYSTEM_NAME}, Target System = ${K3D_TARGET_SYSTEM}")