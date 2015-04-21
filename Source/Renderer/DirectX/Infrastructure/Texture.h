#pragma once 
#include "../Resource.h"
#include "../Helper.h"
namespace k3d
{
  namespace d3d12
  {
    class Texture : public Resource
    {
	public:
		explicit Texture();

		void LoadAndCreate(
			const wchar_t* fileName, 
			UploadBufferWrapper & texUploadWrapper, 
			PtrDevice & device, 
			PtrGfxCmdList & cmdList);
	
    };
  }
}