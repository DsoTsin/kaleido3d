#pragma once
#include "DXCommon.h"
#include "DDSUtil/DDS.h"
#include <Core/Mesh.h>

namespace k3d
{
	struct DxUtil
	{
		static void			ConvertVertexFormatToInputElementDesc(VtxFormat const & format, std::vector<D3D11_INPUT_ELEMENT_DESC> & inputDesc);

		static HRESULT		CreateDDSTextureFromFile(__in ID3D11Device* pDev, __in_z const WCHAR* szFileName, __out_opt ID3D11ShaderResourceView** ppSRV, bool sRGB = false);

		static HRESULT		CreateTextureFromDDS(ID3D11Device* pDev, DDS_HEADER* pHeader, __inout_bcount(BitSize) BYTE* pBitData, UINT BitSize, __out ID3D11ShaderResourceView** ppSRV, bool bSRGB = false);
	};
}