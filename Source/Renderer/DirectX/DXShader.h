#pragma once 

#include <Interface/IShader.h>
#include <Core/Archive.h>
#include <Core/Variant.h>
#include <Core/LogUtil.h>
#include <Core/Mesh.h>

#include "DXCommon.h"
#include "DXShaderDefs.h"
#include "DirectXRenderer.h"

namespace k3d {

	class DXShader : public IShader 
	{
	public:
								DXShader() {}
								DXShader(const char * path, const char* szEntryPoint, const char * szModel) : m_ShaderPath(path), m_ShaderEntryPoint(szEntryPoint), m_ShaderModel(szModel) {}
								~DXShader() { }


		void					LoadCache() override;
		void					SaveCache() override;

		HRESULT					CompileShaderFromFile(D3D_SHADER_MACRO * macro, ID3DInclude * include);
		HRESULT					CompileShaderFromSource(LPCSTR * source, SIZE_T size, LPCSTR szEntryPoint, LPCSTR szShaderModel, D3D_SHADER_MACRO * macro, ID3DInclude * include);

		friend Archive & operator << (Archive & ar, const DXShader & shader)
		{
			assert(shader.m_Blob.Size() != 0);
			ar << shader.m_Blob;
			return ar;
		}

	protected:
		Variant					m_Blob; // dx11 shader needed

	private:
		std::string				m_ShaderPath;
		std::string				m_ShaderEntryPoint;
		std::string				m_ShaderModel;
	};

	typedef std::shared_ptr<DXShader> SpShader;


	DX_SHADER_CLASS_BEGIN(Vertex)

		DxRef::InputLayoutPtr GetInputLayout(VtxFormat const & fmt);

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