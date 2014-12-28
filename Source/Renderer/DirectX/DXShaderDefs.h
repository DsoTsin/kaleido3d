#pragma once

#define DX_SHADER_CLASS_BEGIN(shaderType) \
	class DX##shaderType##Shader : public TShader<ID3D11##shaderType##Shader*, ShaderType::##shaderType##>, public DXShader { \
	public:\
		DX##shaderType##Shader() : TShader() {} \
		~DX##shaderType##Shader() override; \
		HRESULT Init(ID3D11Device *	device, WCHAR * szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel) { \
			HRESULT hr = CompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, nullptr, nullptr, m_Blob);\
			if (FAILED(hr)) {\
				Log::Error("entry %s compile failed!", szEntryPoint);\
				return hr; \
			}\
			hr = device->Create##shaderType##Shader(m_Blob.Data(), m_Blob.Size(), nullptr, &ShaderInst); \
			if (FAILED(hr)) {\
				Log::Error( #shaderType ## " compile failed."); \
				return hr; \
			}\
			return hr;\
		}

#define DX_SHADER_CLASS_END() \
	};



#define RELEASE(shader) \
	if(shader) shader->Release()