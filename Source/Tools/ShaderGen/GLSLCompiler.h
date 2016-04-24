#pragma once
#ifndef __GLSLCompiler_h__
#define __GLSLCompiler_h__

#include "ShaderCompiler.h"
#include <cassert>
#include <vector>

namespace k3d 
{
	class GLSLCompiler : public IShaderCompiler
	{
	public:
									GLSLCompiler();
									~GLSLCompiler() override;
		
		IShaderCompilerOutput*		Compile(ShaderCompilerOption const& option, const char * source) override;
	};


	struct GLSLOutput : public IShaderCompilerOutput
	{
		GLSLOutput() {}
		explicit GLSLOutput(rhi::ShaderByteCode && in) : m_ByteCode(in) {}

		const char*					GetErrorMsg() const { return m_ErrorMsg.c_str(); }
		const char*					GetShaderBytes() const { return reinterpret_cast<const char*>(&m_ByteCode[0]); }
		const uint32				GetByteCount() const { return m_ByteCode.Count()*sizeof(unsigned int); }
		shaderbinding::BindingTable GetBindingTable() const override { return m_BindingTable; }
		const Attributes &			GetAttributes() const { return m_Attributes; }
		const rhi::ShaderByteCode&	GetByteCode() const override 
		{
			return m_ByteCode;
		};

	private:
		friend GLSLOutput*	glslToSpv(	const rhi::EShaderType shader_type,
										const char *pshader);

		rhi::ShaderByteCode						m_ByteCode;
		Attributes								m_Attributes;
		shaderbinding::BindingTable				m_BindingTable;
		std::string								m_ErrorMsg;
		EShaderCompileResult					m_Result;
	};

	extern GLSLOutput*		glslToSpv(	const rhi::EShaderType shader_type,
										const char *pshader);
}

#endif