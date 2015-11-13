#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include <d3dcompiler.h>

NS_K3D_D3D12_BEGIN

const char * ShaderModel[ShaderCompiler::ELangVersion::LangVersionNum][rhi::EShaderType::ShaderTypeNum] =
{
	{ "ps_5_0","vs_5_0","gs_5_0","hs_5_0","ds_5_0","cs_5_0" },
	{ "ps_5_1","vs_5_1","gs_5_1","hs_5_1","ds_5_1","cs_5_1" },
};

rhi::IShaderBytes*	ShaderCompiler::CompileFromSource(ShaderCompiler::ELangVersion LangVersion, rhi::EShaderType ShaderType, const char* Source)
{
	return CompileFromSource(LangVersion, ShaderType, Source, "Main");
}

rhi::IShaderBytes * ShaderCompiler::CompileFromSource(ShaderCompiler::ELangVersion LangVersion, rhi::EShaderType ShaderType, const char* Source, const char * Entry)
{
	PtrBlob ShaderBlob, ErrorBlob;
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT dwShaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT dwShaderFlags = 0;
#endif
	HRESULT Hr = ::D3DCompile(
		Source, strlen(Source), /**SrcData, SrcDataLen***/
		nullptr, nullptr, nullptr, /**SrcName, Defines, Includes**/
		Entry, ShaderModel[LangVersion][ShaderType],
		dwShaderFlags, 0,
		ShaderBlob.GetAddressOf(), ErrorBlob.GetAddressOf());
	if (FAILED(Hr))
	{
		Log::Out(LogLevel::Fatal, "ShaderCompiler", "Failed To Compile: \n%s", Source);
		Log::Out(LogLevel::Fatal, "ShaderCompiler", "Error: %s", ErrorBlob->GetBufferPointer());
		return nullptr;
	}
	Log::Out(LogLevel::Info, "ShaderCompiler", "Succeeded: %s", Source);
	return new ShaderBytes(ShaderBlob);
}

NS_K3D_D3D12_END