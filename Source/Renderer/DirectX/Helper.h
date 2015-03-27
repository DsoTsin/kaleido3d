#pragma once
#include "DXCommon.h"
#include "Infrastructure/Shader.h"
#include "Wrapper.h"
#include <Core>

namespace k3d
{
  namespace d3d12
  {
    /**
    * Simple Helper
    */
    class Helper
    {
    public:
      static void SetResourceBarrier (
        ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* resource,
        UINT StateBefore, UINT StateAfter);

      static void	ConvertVertexFormatToInputElementDesc (
        VtxFormat const & format,
        std::vector<D3D12_INPUT_ELEMENT_DESC> & inputDesc);

      static HRESULT CreateTexture2D (
        _In_ ID3D12Device* d3dDevice,
        _In_ ID3D12GraphicsCommandList* cmdList,
        _In_ UploadBufferWrapper* uploadBuffer,
        _In_ const wchar_t* fileName,
        _Outptr_opt_ ID3D12Resource** resourceOut);

      static void CheckHWFeatures (PtrDevice device);
    };

  }

  namespace d3d11
  {
    /***
    ** Simple Helper for texture loading
    **/
    class Helper
    {
    public:
      static void			ConvertVertexFormatToInputElementDesc (
        VtxFormat const & format,
        std::vector<D3D11_INPUT_ELEMENT_DESC> & inputDesc);

      static HRESULT		CreateDDSTextureFromFile (
        __in ID3D11Device* pDev, __in_z const WCHAR* szFileName,
        __out_opt ID3D11ShaderResourceView** ppSRV, bool sRGB = false);

    
    };
  }
}