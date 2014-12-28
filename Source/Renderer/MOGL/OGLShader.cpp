#include "OGLShader.h"

namespace k3d {

#define CHECK_SHADERPROGRAM_LINKED(prog) \
	::glLinkProgram(prog); \
	GLint success = 0; \
	::glGetProgramiv(prog, GL_LINK_STATUS, &success);\
	if (!success) {\
		char temp[1024];\
		::glGetProgramInfoLog(prog, 1024, 0, temp);\
		Log::Error("Failed to link program:\n%s\n", temp);\
		::glDeleteProgram(prog);\
		(prog) = 0;\
		return nullptr;\
	}

#define CREATE_SHADERPROGRAM_BRANCH_A \
	OGLShaderProgram * prog = new OGLShaderProgram; \
	prog->program = ::glCreateProgram(); \
	if (::glIsProgram(prog->program)) { 
		
#define CREATE_SHADERPROGRAM_BRANCH_B \
	} else { \
		delete prog; \
		prog = nullptr; \
	} \
	return prog;

	OGLShaderProgram * OGLShaderProgram::Link(OGLVertexShader & vShader, OGLPixelShader & pShader) {
		CREATE_SHADERPROGRAM_BRANCH_A
			vShader.AttachTo(prog->program);
			pShader.AttachTo(prog->program);
			CHECK_SHADERPROGRAM_LINKED(prog->program)
		CREATE_SHADERPROGRAM_BRANCH_B
	}

	OGLShaderProgram * OGLShaderProgram::Link(OGLComputeShader & cShader) {
		CREATE_SHADERPROGRAM_BRANCH_A
			cShader.AttachTo(prog->program);
			CHECK_SHADERPROGRAM_LINKED(prog->program)
		CREATE_SHADERPROGRAM_BRANCH_B
	}

	// With Geometry Shader
	OGLShaderProgram * OGLShaderProgram::Link(OGLVertexShader & vShader, OGLGeometryShader & gShader, OGLPixelShader & pShader) {
		CREATE_SHADERPROGRAM_BRANCH_A
				vShader.AttachTo(prog->program);
				gShader.AttachTo(prog->program);
				pShader.AttachTo(prog->program);
			CHECK_SHADERPROGRAM_LINKED(prog->program)
		CREATE_SHADERPROGRAM_BRANCH_B
	}

	// With Tessalation Shader
	OGLShaderProgram * OGLShaderProgram::Link(OGLVertexShader & , OGLHullShader & , OGLDomainShader & , OGLGeometryShader & , OGLPixelShader & ) {
		return nullptr;
	}

	void OGLShaderProgram::Release() {
		if (::glIsProgram(program)) {
			::glDeleteProgram(program);
			program = 0;
		}
	}

}