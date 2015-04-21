#pragma once
#include <KTL/NonCopyable.hpp>
#include "DXCommon.h"
#include "Renderer.h"
#include "Device.h"

namespace k3d
{
  namespace d3d12 
  {
	  /**
	  * RootSignature
	  */
	  class RootSignature
	  {
	  public:
		  void Create(ID3D12Device* device);

		  auto Get() const { return mRootSignature; }
		  auto GetBlob() const { return mBlob; }
		  auto GetErrorBlob() const { return mErrorBlob; }

	  private:
		  PtrRootSignature mRootSignature;
		  PtrBlob mBlob;
		  PtrBlob mErrorBlob;
	  };

  }
}