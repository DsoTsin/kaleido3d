#pragma once 
#include "DXCommon.h"
#include <Engine/Shader.h>
#include <Core/LogUtil.h>
#include <memory>

#include "DXShaderDefs.h"

namespace k3d {

	class DXShader {
	public:
		static HRESULT CompileShaderFromFile(
			WCHAR * szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
			D3D_SHADER_MACRO * macro, ID3DInclude * include,
			Variant & ppBlobOut);
	};

	typedef std::shared_ptr<DXShader> SpShader;


	DX_SHADER_CLASS_BEGIN(Vertex)
		Variant GetBlob() const { return m_Blob; }
	DX_SHADER_CLASS_END()

	DX_SHADER_CLASS_BEGIN(Hull)
	DX_SHADER_CLASS_END()

	DX_SHADER_CLASS_BEGIN(Domain)
	DX_SHADER_CLASS_END()
	
	DX_SHADER_CLASS_BEGIN(Geometry)
	DX_SHADER_CLASS_END()
	
	DX_SHADER_CLASS_BEGIN(Pixel)
	DX_SHADER_CLASS_END()

	DX_SHADER_CLASS_BEGIN(Compute)
	DX_SHADER_CLASS_END()
	
}