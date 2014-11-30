file(GLOB D3D12RHI_SRC_LIST "*.cpp" "*.h")
include_directories(
../.. ../../../Include
.
Utils
)

#if(BUILD_SHARED)
	add_definitions(-DBUILD_SHARED_LIB)
#endif()

set(RHI_HEADERS
	Public/D3D12RHIResource.h
	Public/D3D12RHI.h
)

set(RHI_IMPL_SRCS
	Private/D3D12CommandContext.cpp
	Private/D3D12Device.cpp
	Private/D3D12Resource.cpp
	Private/LinearAllocator.h
	Private/LinearAllocator.cpp
	Private/D3D12Enums.h
	Private/D3D12PipelineState.cpp
	Private/D3D12RootSignature.h
	Private/D3D12RootSignature.cpp
	Private/D3D12Sampler.cpp
	Private/D3D12Compiler.cpp
	Private/D3D12CommandListManager.h
	Private/D3D12CommandListManager.cpp
)

source_group(
	Renderer FILES
	ModuleD3D12.h ModuleD3D12.cpp
	ResourceManager.h
)

source_group(
	Utils FILES
	Color.h
	Utils.h Utils.cpp
	DDSTextureLoader.h DDSTextureLoader.cpp
	DescriptorHeap.h DescriptorHeap.cpp
	PSOLibrary.h PSOLibrary.cpp
	RHI.h RHI.cpp
)

source_group(
	Common FILES
	DXCommon.h
	../IRHI.h
)

source_group(
	Impl\\Public FILES
	${RHI_HEADERS}
)

source_group(
	Impl\\Private FILES
	${RHI_IMPL_SRCS}
)

add_library(RHI_D3D12  ${LIB_TYPE} 
	${D3D12RHI_SRC_LIST} 
	${RHI_HEADERS}
	${RHI_IMPL_SRCS}
)

target_link_libraries(
	RHI_D3D12
	Core Engine 
	d3d12
	d3dcompiler
	dxguid
	dxgi
)

add_precompiled_header(RHI_D3D12 "DXCommon.h" "${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12" "${Kaleido3D_ROOT_DIR}/Source/RHI/D3D12/RHI.cpp")
set_target_properties(RHI_D3D12 PROPERTIES FOLDER "RHI")