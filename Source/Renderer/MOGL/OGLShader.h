#pragma once

#include <Engine/Shader.h>
#include <Core/LogUtil.h>
#include <GL/glew.h>

namespace k3d {

	template <GLuint HWShaderType, ShaderType shaderType>
	class TGLShader : public TShader<GLuint, shaderType> {
	public:
		static const GLuint hwShaderType = HWShaderType;

		// Create Shader here
		bool Compile(const char * source, GLuint & shaderProg) {
			ShaderInst = ::glCreateShader(hwShaderType);
			::glShaderSource(ShaderInst, 1, &source, 0);
			::glCompileShader(ShaderInst);
			GLint success = 0;
			glGetShaderiv(ShaderInst, GL_COMPILE_STATUS, &success);
			if (!success) {
				char temp[1024];
				::glGetShaderInfoLog(ShaderInst, 1024, 0, temp);
				Log::Error("Failed to link program:\n%s\n", temp);
				::glDeleteShader(ShaderInst);
				ShaderInst = GL_INVALID_INDEX;
				return false;
			}
			// AttachTo(shaderProg);
			return true;
		}

		void AttachTo(GLuint prog) {
			::glAttachShader(prog, ShaderInst);
		}

	protected:
		// Shader source copy here
	};

#define	OGL_SHADER_CLASS_BEGIN(shaderType, shaderTypeEnum) \
	class OGL##shaderType##Shader : public TGLShader<##shaderTypeEnum##, ShaderType::##shaderType##> {\
	public:\
		OGL##shaderType##Shader() { ShaderInst = GL_INVALID_INDEX; } \
		~OGL##shaderType##Shader() override { \
			if (::glIsShader(ShaderInst)) { \
				::glDeleteShader(ShaderInst); \
			} \
		}
#define OGL_SHADER_CLASS_END()	};

	OGL_SHADER_CLASS_BEGIN(Vertex, GL_VERTEX_SHADER)
	OGL_SHADER_CLASS_END()

	OGL_SHADER_CLASS_BEGIN(Hull, GL_TESS_CONTROL_SHADER)
	OGL_SHADER_CLASS_END()

	OGL_SHADER_CLASS_BEGIN(Domain, GL_TESS_EVALUATION_SHADER)
	OGL_SHADER_CLASS_END()

	OGL_SHADER_CLASS_BEGIN(Geometry, GL_GEOMETRY_SHADER)
	OGL_SHADER_CLASS_END()

	OGL_SHADER_CLASS_BEGIN(Pixel, GL_FRAGMENT_SHADER)
	OGL_SHADER_CLASS_END()

	OGL_SHADER_CLASS_BEGIN(Compute, GL_COMPUTE_SHADER)
	OGL_SHADER_CLASS_END()

	class OGLShaderProgram {
	public:

		static OGLShaderProgram * Link(OGLVertexShader & vShader, OGLPixelShader & pShader);

		static OGLShaderProgram * Link(OGLComputeShader & cShader);

		// With Geometry Shader
		static OGLShaderProgram * Link(OGLVertexShader & vShader, OGLGeometryShader & gShader, OGLPixelShader & pShader);

		// With Tessalation Shader
		static OGLShaderProgram * Link(OGLVertexShader & vShader, OGLHullShader & hShader, OGLDomainShader & dShader, OGLGeometryShader & gShader, OGLPixelShader & pShader);

		void Release();

	protected:
		OGLShaderProgram() = default;

		friend class Shader;
	private:
		GLuint program;
	};
}