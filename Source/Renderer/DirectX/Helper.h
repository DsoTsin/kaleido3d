#pragma once
#include "DXCommon.h"
#include "DDSUtil/DDS.h"
#include <Core>

namespace k3d
{
  namespace d3d12
  {
    class Helper
    {
    public:
      static void SetResourceBarrier (ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, UINT StateBefore, UINT StateAfter);
      static void	ConvertVertexFormatToInputElementDesc (VtxFormat const & format, std::vector<D3D12_INPUT_ELEMENT_DESC> & inputDesc);
    };
  }

  namespace d3d11
  {
    class Helper
    {
    public:
      static void			ConvertVertexFormatToInputElementDesc (VtxFormat const & format, std::vector<D3D11_INPUT_ELEMENT_DESC> & inputDesc);
      static HRESULT		CreateDDSTextureFromFile (__in ID3D11Device* pDev, __in_z const WCHAR* szFileName, __out_opt ID3D11ShaderResourceView** ppSRV, bool sRGB = false);
      static HRESULT		CreateTextureFromDDS (ID3D11Device* pDev, DDS_HEADER* pHeader, __inout_bcount (BitSize) BYTE* pBitData, UINT BitSize, __out ID3D11ShaderResourceView** ppSRV, bool bSRGB = false);
    };
  }
}