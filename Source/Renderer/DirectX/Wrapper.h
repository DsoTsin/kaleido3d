#pragma once
#include "DXCommon.h"

namespace k3d {
	namespace d3d12 {
		class UploadBufferWrapper
		{
		public:
			HRESULT Create(
				ID3D12Device* device,
				const SIZE_T size, D3D12_HEAP_TYPE heapType,
				D3D12_HEAP_FLAGS miscFlag = D3D12_HEAP_FLAG_NONE);

			PtrResource pBuf;
			UINT8* pDataBegin;
			UINT8* pDataCur;
			UINT8* pDataEnd;
		};

		/**
		* DescriptorHeapWrapper
		*/
		class DescriptorHeapWrapper
		{
		public:
			DescriptorHeapWrapper()
			{
				memset(this, 0, sizeof(*this));
			}

			HRESULT Create(
				ID3D12Device* pDevice,
				D3D12_DESCRIPTOR_HEAP_TYPE Type,
				UINT NumDescriptors,
				bool bShaderVisible = false);

			operator ID3D12DescriptorHeap*()
			{
				return pDH;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE hCPU(UINT index)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE handle;
				handle.ptr = MakeOffsetted(hCPUHeapStart.ptr, index);
				return handle;
			}

			D3D12_GPU_DESCRIPTOR_HANDLE hGPU(UINT index)
			{
				assert(Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
				D3D12_GPU_DESCRIPTOR_HANDLE handle;
				handle.ptr = MakeOffsetted(hGPUHeapStart.ptr, index);
				return handle;
			}

			SIZE_T MakeOffsetted(SIZE_T ptr, UINT index)
			{
				SIZE_T offsetted;
				offsetted = ptr + index * HandleIncrementSize;
				return offsetted;
			}

			D3D12_DESCRIPTOR_HEAP_DESC Desc;
			PtrDescHeap pDH;
			D3D12_CPU_DESCRIPTOR_HANDLE hCPUHeapStart;
			D3D12_GPU_DESCRIPTOR_HANDLE hGPUHeapStart;
			UINT HandleIncrementSize;
		};
	}
}