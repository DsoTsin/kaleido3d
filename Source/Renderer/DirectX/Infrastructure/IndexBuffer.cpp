#include "Kaleido3D.h"
#include "IndexBuffer.h"

namespace k3d
{
  namespace d3d12
  {
	  void IndexBuffer::Create(
		  ID3D12Device * device, 
		  int32_t sizeInBytes,
		  DXGI_FORMAT format,
		  const void * data)
	  {
		  _Create(device, sizeInBytes, data);
		  m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
		  m_View.SizeInBytes = sizeInBytes;
		  m_View.Format = format;
	  }
  }
}