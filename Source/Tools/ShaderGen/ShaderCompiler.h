#ifndef __ShaderGen_h__
#define __ShaderGen_h__

#include <RHI/IRHIDefs.h>
#include <Config/Config.h>
#include <string>

namespace k3d 
{
	namespace shaderbinding
	{
		enum class ESemantic
		{
			EPOSITION,
			ECOLOR,
			ENORMAL,
			ENumSemanics
		};

		enum class EDataType
		{
			EBool, EBool2, EBool3, EBool4,
			EInt, EInt2, EInt3, EInt4,
			EUInt, EUInt2, EUInt3, EUInt4,
			EFloat, EFloat2, EFloat3, EFloat4,
			EMat2, EMat2x3, EMat2x4, EMat3x2, EMat3, EMat3x4, EMat4x2, EMat4x3, EMat4,
			EUnknown
		};

		struct Attribute
		{
			std::string	VarName;
			ESemantic	VarSemantic;
			EDataType	VarType;
			uint32		VarLocation;
			uint32		VarBindingPoint;
			uint32		VarCount;
		};

		struct Uniform
		{

		};
	}

	class IShaderCompilerOutput
	{
	public:
		virtual const char*					GetErrorMsg() const = 0;
		virtual const char*					GetShaderBytes() const = 0;
		virtual const uint32				GetByteCount() const = 0;
		virtual const rhi::ShaderByteCode&	GetByteCode() const = 0;
		virtual								~IShaderCompilerOutput() {}
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
		virtual void Reflect(const char* byteCode, uint32 length) {}
		virtual IShaderCompilerOutput * Compile(ShaderCompilerOption const& option, const char * source) = 0;
		virtual ~IShaderCompiler() {}
	};
}

extern K3D_API ::k3d::IShaderCompiler * CreateShaderCompiler(::k3d::EShaderLang lang);

#endif