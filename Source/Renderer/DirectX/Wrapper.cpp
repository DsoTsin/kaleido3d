#include "Kaleido3D.h"
#include "Wrapper.h"

namespace k3d {
	namespace d3d12 {
		HRESULT UploadBufferWrapper::Create(
			ID3D12Device * device,
			const SIZE_T size,
			D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS miscFlag)
		{
			D3D12_HEAP_PROPERTIES heapProps;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.Type = heapType;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC bufferDesc;
			bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufferDesc.Alignment = 0;
			bufferDesc.Width = static_cast<UINT64>(size);
			bufferDesc.Height = 1;
			bufferDesc.DepthOrArraySize = 1;
			bufferDesc.MipLevels = 1;
			bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.SampleDesc.Quality = 0;
			bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			HRESULT hr = device->CreateCommittedResource(
				&heapProps,
				miscFlag,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(pBuf.GetInitReference()));

			if (hr == S_OK)
			{
				pBuf->Map(0, nullptr, (void**)&pDataBegin);
				pDataCur = pDataBegin;
				pDataEnd = pDataBegin + size;
			}

			return hr;
		}

		HRESULT DescriptorHeapWrapper::Create(
			ID3D12Device * pDevice,
			D3D12_DESCRIPTOR_HEAP_TYPE Type,
			UINT NumDescriptors, bool bShaderVisible)
		{
			Desc.Type = Type;
			Desc.NumDescriptors = NumDescriptors;
			Desc.Flags = (bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : (D3D12_DESCRIPTOR_HEAP_FLAGS)0);
			HRESULT hr = pDevice->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(pDH.GetInitReference()));
			if (FAILED(hr))
				return hr;

			hCPUHeapStart = pDH->GetCPUDescriptorHandleForHeapStart();
			if (bShaderVisible)
			{
				hGPUHeapStart = pDH->GetGPUDescriptorHandleForHeapStart();
			}
			else
			{
				hGPUHeapStart.ptr = 0;
			}
			HandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(Desc.Type);
			return hr;
		}

	}
}