#ifndef __ShaderGen_h__
#define __ShaderGen_h__

#include <RHI/IRHIDefs.h>
#include <Config/Config.h>
#include <string>

namespace k3d 
{

	class IShaderCompilerOutput
	{
	public:
		virtual const char*			GetErrorMsg() const = 0;
		virtual const char*			GetShaderBytes() const = 0;
		virtual const uint32		GetByteCount() const = 0;
		virtual						~IShaderCompilerOutput() {}
	};

	enum class EShaderLang
	{
		HLSL,
		GLSL,
		MetalSL
	};

	enum class EShaderCompileResult
	{
		Success,
		Fail
	};
	
	struct ShaderCompilerOption
	{
		rhi::EShaderType	ShaderType;
		std::string			ShaderModel;
		std::string			EntryFunction;
	};

	class K3D_API IShaderCompiler
	{
	public:
		virtual IShaderCompilerOutput * Compile(ShaderCompilerOption const& option, const char * source) = 0;
		virtual ~IShaderCompiler() {}
	};
}

extern K3D_API ::k3d::IShaderCompiler * CreateShaderCompiler(::k3d::EShaderLang lang);

#endif