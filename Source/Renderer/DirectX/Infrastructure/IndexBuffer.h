#pragma once 
#include "../Resource.h"

namespace k3d
{
  namespace d3d12
  {
    class IndexBuffer : public CommittedResource
    {
    public:
		void Create(
			ID3D12Device* device,
			int32_t sizeInBytes,
			DXGI_FORMAT format,
			_In_opt_ const void* data);

		const auto& GetView() const { return m_View; }
	private:
		D3D12_INDEX_BUFFER_VIEW m_View;
    };
  }
}