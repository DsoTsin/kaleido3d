#pragma once

#define DX_SHADER_CLASS_BEGIN(shaderType) \
	class DX##shaderType##Shader : public TShader<Ref<ID3D11##shaderType##Shader>, ShaderType::##shaderType##>, public virtual DXShader { \
	public:\
		DX##shaderType##Shader(const char * path, const char* szEntryPoint, const char * szModel) : TShader(), DXShader(path, szEntryPoint, szModel) {} \
		~DX##shaderType##Shader() override; \
		HRESULT Init() { \
			HRESULT hr = CompileShaderFromFile(nullptr, nullptr);\
			if (FAILED(hr)) {\
				return hr; \
			}\
			hr = DXDevice::Get().Device()->Create##shaderType##Shader(m_Blob.Data(), m_Blob.Size(), nullptr, ShaderInst.GetInitReference()); \
			if (FAILED(hr)) {\
				Log::Error( #shaderType ## " compile failed."); \
				return hr; \
			}\
			return hr;\
		}

#define DX_SHADER_CLASS_END() \
	};

