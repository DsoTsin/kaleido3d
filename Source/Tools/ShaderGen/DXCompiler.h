#pragma once
#ifndef __DXCompiler_h__
#define __DXCompiler_h__

namespace k3d {

	class DXCompiler : public IShaderCompiler
	{
	public:

		DXCompiler() {}
		~DXCompiler() override {}

		IShaderCompilerOutput * Compile(ShaderCompilerOption const& option, const char * source) override;
	};

	class DXCompilerOutput : public IShaderCompilerOutput
	{
		friend class DXCompiler;
	public:
		DXCompilerOutput() {}
		~DXCompilerOutput() override {}

		const char*		GetErrorMsg() const		{ return m_ErrorMsg.c_str(); }
		const char*		GetShaderBytes() const	{ return (const char*)m_Data.Data(); }
		const uint32	GetByteCount() const	{ return m_Data.Count()*sizeof(uint32); }

		const rhi::ShaderByteCode& GetByteCode() const override
		{
			return m_Data;
		};

	private:
		rhi::ShaderByteCode		m_Data;
		std::string				m_ErrorMsg;
		EShaderCompileResult	m_Result;
	};
}

#endif