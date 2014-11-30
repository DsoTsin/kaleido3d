#include "Function.hpp"

#include <GL/glew.h>

Function::Function() :
	functionType((FunctionType)0),
	handle(GL_INVALID_INDEX)
{}

Function::~Function() {
	if (handle != GL_INVALID_INDEX) {
		glDeleteShader(handle);
		handle = GL_INVALID_INDEX;
	}
}

bool Function::compileShader() {
	if (handle == GL_INVALID_INDEX) {
		handle = glCreateShader((GLenum)functionType);
		const char * shader = source.c_str();
		GLint length = (GLint)source.length();
		glShaderSource(handle, 1, &shader, &length);
		glCompileShader(handle);
		GLint compiled = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			if (!compiled) {
				printf("Error compiling shader");
			}
			GLint infoLen = 0;
			glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				char* buf = new char[infoLen];
				if (buf) {
					glGetShaderInfoLog(handle, infoLen, NULL, buf);
					printf("Shader log:\n%s\n", buf);
					delete[] buf;
				}
			}
			if (!compiled) {
				glDeleteShader(handle);
				handle = GL_INVALID_INDEX;
				return false;
			}
		}
	}
	return true;
}