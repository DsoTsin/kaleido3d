#include "Kaleido3D.h"
#include "Texture.h"

namespace k3d
{
  namespace d3d12
  {
	  Texture::Texture()
	  {
	  }

	  void Texture::LoadAndCreate(
		  const wchar_t * fileName, 
		  UploadBufferWrapper & texUploadWrapper, 
		  PtrDevice & device, 
		  PtrGfxCmdList & cmdList)
	  {
		  HRESULT hr = Helper::CreateTexture2D(
			  device,
			  cmdList,
			  &texUploadWrapper,
			  fileName,
			  m_Resource.GetInitReference());
	  }
  }
}