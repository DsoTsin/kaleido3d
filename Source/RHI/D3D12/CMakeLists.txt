file(GLOB D3D12RHI_SRC_LIST "*.cpp" "*.h")
include_directories(
../.. ../../../Include
.
Utils
${DXSDK_INCLUDE_DIR}
)

#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

set(RHI_HEADERS
	../IRHI.h
	Public/D3D12RHI.h
	Public/D3D12Viewport.h
	Public/D3D12RenderTarget.h
)

set(RHI_IMPL_SRCS
	Private/D3D12CommandContext.cpp
	Private/D3D12Device.cpp
	Private/D3D12Resource.cpp
	Private/LinearAllocator.cpp
	Private/D3D12Enums.h
	Private/D3D12PipelineState.cpp
	Private/D3D12RootSignature.h
	Private/D3D12RootSignature.cpp
	Private/D3D12Sampler.cpp
	Private/D3D12CommandListManager.h
	Private/D3D12CommandListManager.cpp
	Private/D3D12Viewport.cpp
	Private/D3D12RenderTarget.cpp
	Private/D3D12GpuBuffer.h
	Private/D3D12GpuTexture.h
)

source_group(
	Utils FILES
	Color.h
	Utils.h Utils.cpp
	DDSTextureLoader.h DDSTextureLoader.cpp
	PSOLibrary.h PSOLibrary.cpp
	RHI.h RHI.cpp
)

source_group(
	Common FILES
	DXCommon.h
	../IRHI.h
)

source_group(
	Public FILES
	${RHI_HEADERS}
)

source_group(
	Private FILES
	${RHI_IMPL_SRCS}
)

add_library(RHI_D3D12  ${LIB_TYPE} 
	${D3D12RHI_SRC_LIST} 
	${RHI_HEADERS}
	${RHI_IMPL_SRCS}
)

target_link_libraries(
	RHI_D3D12
	Core Engine ShaderCompiler
	${DXSDK_LIBRARIES}
)

add_precompiled_header(RHI_D3D12 "DXCommon.h" "${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12" "${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12/RHI.cpp")
set_target_properties(RHI_D3D12 PROPERTIES FOLDER "Runtime/RHI")

install(TARGETS RHI_D3D12 ARCHIVE DESTINATION lib)