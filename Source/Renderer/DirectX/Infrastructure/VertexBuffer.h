#pragma once 
#include "../Resource.h"

namespace k3d
{
  namespace d3d12
  {
	class VertexBuffer : public CommittedResource
	{
	public:
		void Create(
			ID3D12Device* device,
			int32_t sizeInBytes,
			int32_t strideInBytes,
			_In_opt_ const void* data);

		auto GetStride() const { return m_Stride; }
		const auto& GetView() const { return m_View; }

	private:
		int32_t m_Stride;
		D3D12_VERTEX_BUFFER_VIEW m_View;
	};

    class VertexBufferCompletedHandler
    {
      public:
        virtual void OnComplete(VertexBuffer & vb) = 0;
    };
  }
}