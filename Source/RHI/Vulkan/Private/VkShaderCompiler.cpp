#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"

K3D_VK_BEGIN

rhi::IShaderBytes*	Compiler::CompileFromSource(Compiler::ELangVersion LangVersion, rhi::EShaderType ShaderType, const char* Source)
{
	return CompileFromSource(LangVersion, ShaderType, Source, "Main");
}

rhi::IShaderBytes * Compiler::CompileFromSource(Compiler::ELangVersion LangVersion, rhi::EShaderType ShaderType, const char* Source, const char * Entry)
{
	return nullptr;
}

K3D_VK_END