#pragma once
#ifndef __GLSLCompiler_h__
#define __GLSLCompiler_h__

#include "ShaderCompiler.h"
#include <cassert>
#include <vector>

namespace k3d {

	class GLSLReflector
	{
	public:
		GLSLReflector();
		~GLSLReflector();
	};

	class GLSLCompiler : public IShaderCompiler
	{
	public:
									GLSLCompiler();
									~GLSLCompiler() override;
		/**
		 * @param byteCode SPIR-V ByteCode
		 */
		virtual void				Reflect(const char* byteCode, uint32 length) override;
		IShaderCompilerOutput *		Compile(ShaderCompilerOption const& option, const char * source) override;
	};

	struct GLSLOutput : public IShaderCompilerOutput
	{
		GLSLOutput() {}
		explicit GLSLOutput(std::vector<unsigned int> && in) : shaderOutput(in) {}
		explicit GLSLOutput(rhi::ShaderByteCode && in) : m_ByteCode(in) {}

		const char*			GetErrorMsg() const { return m_ErrorMsg.c_str(); }
		const char*			GetShaderBytes() const { return reinterpret_cast<const char*>(&m_ByteCode[0]); }
		const uint32		GetByteCount() const { return m_ByteCode.Count()*sizeof(unsigned int); }

		const rhi::ShaderByteCode& GetByteCode() const override 
		{
			return m_ByteCode;
		};

	private:
		friend GLSLOutput*	glslToSpv(	const rhi::EShaderType shader_type,
										const char *pshader);

		std::vector<unsigned int> shaderOutput;
		rhi::ShaderByteCode		m_ByteCode;
		std::string				m_ErrorMsg;
		EShaderCompileResult	m_Result;
	};

	extern GLSLOutput*		glslToSpv(	const rhi::EShaderType shader_type,
										const char *pshader);
}

#endif