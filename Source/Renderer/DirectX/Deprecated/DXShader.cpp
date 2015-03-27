#include "Kaleido3D.h"
#include "DXUtil.h"
#include "DXShader.h"
#include "DXShaderManager.h"


#include <Core/LogUtil.h>
#include <Core/File.h>
#include <Core/AssetManager.h>
#include <Core/Utils/StringUtils.h>

namespace k3d {
	
	void DXShader::LoadCache()
	{
		if (m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()) {
			Debug::Out("DXShader","LoadCache---DXShader is invalid: \"m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()\"");
			return;
		}

		if (m_Blob.Size() > 0)
		{
			Debug::Out("DXShader","LoadCache--- cache already loaded.");
			return;
		}

		std::string cachePath = GenerateShaderCachePath(m_ShaderPath.c_str(), m_ShaderEntryPoint.c_str(), m_ShaderModel.c_str());
		AssetManager::SpIODevice cacheDevice = AssetManager::OpenAsset(cachePath.c_str());
		if (cacheDevice)
		{
			int64 length = std::static_pointer_cast<File>(cacheDevice)->GetSize();
			char * bytes = new char[length];
			cacheDevice->Read(bytes, length);
			m_Blob.DeepCopy(bytes, length, true);
			cacheDevice->Close();
			Log::Message("shader cache loaded! size=%ld", length);
			Log::Message("DXShader::LoadCache--- shader cache(%s) loaded!", cachePath.c_str());
		}
	}

	void DXShader::SaveCache()
	{
		if (m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()) {
			Debug::Out("DXShader","SaveCache---DXShader is invalid: \"m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()\"\n");
			return;
		}
		if (m_Blob.Size() == 0)
		{
			Debug::Out("DXShader","SaveCache--- no cache generated..\n");
			return;
		}

		std::string cachePath = GenerateShaderCachePath(m_ShaderPath.c_str(), m_ShaderEntryPoint.c_str(), m_ShaderModel.c_str());
		AssetManager::SpIODevice cacheDevice = AssetManager::OpenAsset(cachePath.c_str(), IOWrite);
		if(cacheDevice)
		{
			cacheDevice->Write(m_Blob.Data(), m_Blob.Size());
			cacheDevice->Close();
			Log::Message("DXShader::SaveCache--- shader cache saved(%s)!", cachePath.c_str());
		}
	}

	HRESULT DXShader::CompileShaderFromFile(D3D_SHADER_MACRO * macro, ID3DInclude * include)
	{
		if (m_Blob.Size() > 0)
		{
			Debug::Out("DXShader","CompileShaderFromFile (%s) already loaded..", m_ShaderPath.c_str());
			return S_OK;
		}

		if (m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()) {
			Debug::Out("DXShader","CompileShaderFromFile---DXShader is invalid: \"m_ShaderPath.empty() || m_ShaderEntryPoint.empty() || m_ShaderModel.empty()\"");
			return S_FALSE;
		}

		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if (_WIN32_WINNT <= _WIN32_WINNT_WIN8)
#ifdef _DEBUG
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
#endif
		std::string realPath = AssetManager::AssetPath(m_ShaderPath.c_str());
		// Generate Shader Cache Path
		std::string cCachePath = GenerateShaderCachePath(realPath.c_str(), m_ShaderEntryPoint.c_str(), m_ShaderModel.c_str());

		// First: Check shader cache
		{
			File cache;
			if (cache.Open(cCachePath.c_str(), IORead)) {
				int64 length = cache.GetSize();
				char * bytes = new char[length];
				cache.Read(bytes, length);
				m_Blob.DeepCopy(bytes, length, true);
				cache.Close();
				Log::Message("shader cache loaded! size=%ld", length);
				return S_OK;
			}
		}

		Ref<ID3DBlob> pBlobOut;
		Ref<ID3DBlob> pErrorBlob;
		
		wchar_t szFileName[256] = { 0 };
		StringUtil::CharToWchar(realPath.c_str(), szFileName, sizeof(szFileName));

		hr = D3DCompileFromFile(szFileName, macro, include, m_ShaderEntryPoint.c_str(), m_ShaderModel.c_str(), dwShaderFlags, 0, pBlobOut.GetInitReference(), pErrorBlob.GetInitReference());

		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
				Log::Error("Shader failed to compile. %s", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			}
			Debug::Out("DXShader","CompileShaderFromFile-- Failed to Compile (%s).", m_ShaderPath.c_str());
			return hr;
		}

		Log::Message("Shader Compile Completed!");
		
		m_Blob.DeepCopy(pBlobOut->GetBufferPointer(), pBlobOut->GetBufferSize());
		
		// Now : save shader cache
		{
			File cache;
			if (cache.Open(cCachePath.c_str(), IOWrite)) {
				cache.Write(m_Blob.Data(), m_Blob.Size());
				cache.Close();
				Log::Message("shader cache saved!");
			}
		}

		return S_OK;
	}

	HRESULT DXShader::CompileShaderFromSource(LPCSTR * source, SIZE_T size, LPCSTR szEntryPoint, LPCSTR szShaderModel, D3D_SHADER_MACRO * macro, ID3DInclude * include)
	{
		if (m_Blob.Size() > 0)
		{
			Debug::Out("DXShader","CompileShaderFromSource (%s) already loaded.\n", source);
			return S_OK;
		}

		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if (_WIN32_WINNT <= _WIN32_WINNT_WIN8)
#ifdef _DEBUG
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
#endif

		ID3DBlob * _ppBlobOut = nullptr;
		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompile(source, size, nullptr, macro, include, szEntryPoint, szShaderModel, dwShaderFlags, 0, &_ppBlobOut, &pErrorBlob);
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

		m_Blob.DeepCopy(_ppBlobOut->GetBufferPointer(), _ppBlobOut->GetBufferSize());
		_ppBlobOut->Release();

		return S_OK;
	}
	
	DXVertexShader::~DXVertexShader() {
	}

	DXHullShader::~DXHullShader() {
	}

	DXDomainShader::~DXDomainShader() {
	}

	DXGeometryShader::~DXGeometryShader() {
	}

	DXPixelShader::~DXPixelShader() {
	}

	DXComputeShader::~DXComputeShader() {
	}

	DxRef::InputLayoutPtr DXVertexShader::GetInputLayout(VtxFormat const & fmt)
	{
		DxRef::InputLayoutPtr pVertexLayout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc;
		DxUtil::ConvertVertexFormatToInputElementDesc(fmt, inputDesc);
		assert(inputDesc.size() > 0 && m_Blob.Size()>0);
		if (FAILED(DXDevice::Get().Device()->CreateInputLayout(
			&inputDesc[0], (UINT)inputDesc.size(), m_Blob.Data(), m_Blob.Size(), pVertexLayout.GetInitReference())
			))
		{
			Log::Fatal("DXRenderMesh::Init device.Device()->CreateInputLayout failed..");
		}
		return pVertexLayout;
	}
}