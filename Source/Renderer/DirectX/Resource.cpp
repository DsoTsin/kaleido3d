#include "Kaleido3D.h"
#include "Resource.h"
#include <Core>

namespace k3d {
	namespace d3d12 {

		void CommittedResource::_Create(
			ID3D12Device* device,
			int64_t size,
			_In_opt_ const void* data)
		{
			D3D12_HEAP_PROPERTIES heapProps;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
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

			m_Size = size;
			ThrowIfFailed(
				device->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(m_Resource.GetInitReference()))
				);

			if (data)
			{
				UploadData(data, size);
			}
		}

		void CommittedResource::UploadData(const void* data, int64_t size) {
			UINT8* dataBegin;
			ThrowIfFailed(m_Resource->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin)));
			memcpy(dataBegin, data, static_cast<size_t>(size));
			m_Resource->Unmap(0, nullptr);
		}

	}
}