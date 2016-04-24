#pragma once
#ifndef __DXCompiler_h__
#define __DXCompiler_h__

#if K3DPLATFORM_OS_WIN
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

		const char*					GetErrorMsg() const					{ return m_ErrorMsg.c_str(); }
		const char*					GetShaderBytes() const				{ return (const char*)m_Data.Data(); }
		const uint32				GetByteCount() const				{ return m_Data.Count()*sizeof(uint32); }
		shaderbinding::BindingTable GetBindingTable() const override	{ return m_BindingTable; }
		const Attributes&			GetAttributes() const override		{ return m_Attributes;}
		const rhi::ShaderByteCode&	GetByteCode() const override		{ return m_Data; };

	private:
		rhi::ShaderByteCode			m_Data;
		shaderbinding::BindingTable	m_BindingTable;
		Attributes					m_Attributes;
		std::string					m_ErrorMsg;
		EShaderCompileResult		m_Result;
	};
}
#endif
#endif