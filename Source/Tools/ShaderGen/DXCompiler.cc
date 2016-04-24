#include "ShaderCompiler.h"

#if K3DPLATFORM_OS_WIN
#include "DXCompiler.h"
#include <d3dcompiler.h>
#include <wrl/client.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace k3d {

	IShaderCompilerOutput * DXCompiler::Compile(ShaderCompilerOption const& option, const char * source)
	{
		using PtrBlob = Microsoft::WRL::ComPtr<ID3DBlob>;
		PtrBlob ShaderBlob, ErrorBlob;
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT dwShaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT dwShaderFlags = 0;
#endif
		HRESULT Hr = ::D3DCompile(
			source, strlen(source), /**SrcData, SrcDataLen***/
			nullptr, nullptr, nullptr, /**SrcName, Defines, Includes**/
			option.EntryFunction.c_str(), option.ShaderModel.c_str(),
			dwShaderFlags, 0,
			ShaderBlob.GetAddressOf(), ErrorBlob.GetAddressOf());

		DXCompilerOutput * output = new DXCompilerOutput;
		
		if (FAILED(Hr))
		{
			output->m_ErrorMsg = { (const char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize() };
			output->m_Result = EShaderCompileResult::Fail;
			return output;
		} 
		else 
		{
			output->m_Data = { (const char*)ShaderBlob->GetBufferPointer(), (uint32)ShaderBlob->GetBufferSize() };
			output->m_Result = EShaderCompileResult::Success;
			return output;
		}
	}



}
#endif