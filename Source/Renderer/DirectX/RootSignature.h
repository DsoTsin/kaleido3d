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

		  auto Get() const { return          m_RootSignature; }
		  auto GetBlob() const { return      m_Blob; }
		  auto GetErrorBlob() const { return m_ErrorBlob; }

	  private:
		  PtrRootSignature                   m_RootSignature;
		  PtrBlob                            m_Blob;
		  PtrBlob                            m_ErrorBlob;
	  };

  }
}