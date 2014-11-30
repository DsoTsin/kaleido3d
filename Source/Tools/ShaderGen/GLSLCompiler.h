#pragma once
#ifndef __GLSLCompiler_h__
#define __GLSLCompiler_h__

#include "ShaderCompiler.h"
#include <vector>

namespace k3d {

	class GLSLCompiler : public IShaderCompiler
	{
	public:
		GLSLCompiler();
		~GLSLCompiler() override;

		IShaderCompilerOutput * Compile(ShaderCompilerOption const& option, const char * source) override;
	};

	struct GLSLOutput : public IShaderCompilerOutput
	{
		GLSLOutput() {}
		explicit GLSLOutput(std::vector<unsigned int> && in) : shaderOutput(in) {}

		const char*			GetErrorMsg() const { return m_ErrorMsg.c_str(); }
		const char*			GetShaderBytes() const { return reinterpret_cast<const char*>(&shaderOutput[0]); }
		const uint32		GetByteCount() const { return shaderOutput.size()*sizeof(unsigned int); }

	private:
		friend GLSLOutput*	glslToSpv(	const rhi::EShaderType shader_type,
										const char *pshader);

		std::vector<unsigned int> shaderOutput;
		std::string				m_ErrorMsg;
		EShaderCompileResult	m_Result;
	};

	extern GLSLOutput*		glslToSpv(	const rhi::EShaderType shader_type,
										const char *pshader);
}

#endif