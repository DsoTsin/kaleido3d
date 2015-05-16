#include "Kaleido3D.h"
#include "VertexBuffer.h"

namespace k3d
{
  namespace d3d12
  {
	  void VertexBuffer::Create(
		  ID3D12Device* device,
		  int32_t sizeInBytes,
		  int32_t strideInBytes,
		  _In_opt_ const void* data)
	  {
		  m_Stride = strideInBytes;
		  _Create(device, sizeInBytes, data);
		  m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
		  m_View.SizeInBytes = sizeInBytes;
		  m_View.StrideInBytes = strideInBytes;
	  }
  }
}