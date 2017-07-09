
# Try to find DirectX SDK.
# Once done this will define
#
# DXSDK_FOUND
# DXSDK_INCLUDE_DIR
# DXSDK_LIBRARY_DIR
# DXSDK_LIBRARIES
# DXSDK_LOCATION
#
# Also will define

if (WIN32)
    find_path(DXSDK_INCLUDE_DIR
        NAMES
            D3D11.h D3Dcompiler.h D3D12.h
        PATH_SUFFIXES Include
        PATHS
            ${DXSDK_LOCATION}
            $ENV{DXSDK_LOCATION}
            ${DXSDK_ROOT}
            $ENV{DXSDK_ROOT}
            ${K3D_THIRD_PARTY}/dxsdk
            "C:/Program Files (x86)/Microsoft DirectX SDK*"
            "C:/Program Files/Microsoft DirectX SDK*"
    )

    find_path(DXSDK_LIBRARY_DIR
        NAMES
            d3d11.lib d3d12.lib
        PATH_SUFFIXES Lib/x64
        PATHS
            ${DXSDK_LOCATION}
            $ENV{DXSDK_LOCATION}
            ${DXSDK_ROOT}
            $ENV{DXSDK_ROOT}
            ${K3D_THIRD_PARTY}/dxsdk
            "C:/Program Files (x86)/Microsoft DirectX SDK*/"
            "C:/Program Files/Microsoft DirectX SDK*/"
    )

    foreach(DX_LIB d3d11 d3d12 d3dcompiler d3dx11 dxguid dxgi)

        find_library(DXSDK_${DX_LIB}_LIBRARY
            NAMES 
                ${DX_LIB}.lib
            PATHS
                ${DXSDK_LIBRARY_DIR}
        )

        list(APPEND DXSDK_LIBRARIES ${DXSDK_${DX_LIB}_LIBRARY})

    endforeach(DX_LIB)

endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DXSDK DEFAULT_MSG
    DXSDK_INCLUDE_DIR
    DXSDK_LIBRARY_DIR
    DXSDK_LIBRARIES
)

mark_as_advanced(
    DXSDK_INCLUDE_DIR
    DXSDK_LIBRARY_DIR
    DXSDK_LIBRARIES
)