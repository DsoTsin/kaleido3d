#include "DXShader.h"
#include "DXShaderManager.h"
#include <Core/LogUtil.h>
#include <Core/File.h>
#include <string>
#include <functional>

#include <Core/Utils/StringUtils.h>

namespace k3d {


	HRESULT DXShader::CompileShaderFromFile(
		WCHAR * szFileName,
		LPCSTR szEntryPoint, LPCSTR szShaderModel,
		D3D_SHADER_MACRO * macro, ID3DInclude * include,
		Variant & ppBlobOut)
	{
		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		WCHAR smName[128] = { 0 };
		CharToWchar(szShaderModel, smName, sizeof(smName));
		std::wstring hashPath = PathToHash(szFileName).append(smName);
		// First: Check shader cache
		{
			File cache;
			if (cache.Open(hashPath.c_str(), IORead)) {
				int64 length = cache.GetSize();
				char * bytes = new char[length];
				cache.Read(bytes, length);
				ppBlobOut.DeepCopy(bytes, length, true);
				cache.Close();
				Log::Message("shader cache loaded! size=%ld", length);
				return S_OK;
			}
		}
 
		ID3DBlob * _ppBlobOut = nullptr;
		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(szFileName, macro, include, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, &_ppBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
				Log::Error("Shader failed to compile. %s", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
				pErrorBlob->Release();
			}
			return hr;
		}

		Log::Message("Shader Compile Completed!");
		
		if (pErrorBlob) {
			pErrorBlob->Release();
		}

		ppBlobOut.DeepCopy(_ppBlobOut->GetBufferPointer(), _ppBlobOut->GetBufferSize());
		_ppBlobOut->Release();
		
		// Now : save shader cache
		{
			File cache;
			if (cache.Open(hashPath.c_str(), IOWrite)) {
				cache.Write(ppBlobOut.Data(), ppBlobOut.Size());
				cache.Close();
				Log::Message("shader cache saved!");
			}
		}

		return S_OK;
	}

	DXVertexShader::~DXVertexShader() {
		RELEASE(ShaderInst);
	}

	DXHullShader::~DXHullShader() {
		RELEASE(ShaderInst);
	}

	DXDomainShader::~DXDomainShader() {
		RELEASE(ShaderInst);
	}

	DXGeometryShader::~DXGeometryShader() {
		RELEASE(ShaderInst);
	}

	DXPixelShader::~DXPixelShader() {
		RELEASE(ShaderInst);
	}

	DXComputeShader::~DXComputeShader() {
		RELEASE(ShaderInst);
	}
}