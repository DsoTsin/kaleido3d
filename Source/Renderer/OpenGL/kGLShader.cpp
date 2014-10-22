#include "kGLShader.h"

#include <Core/k3dLog.h>
#include <Core/k3dDbg.h>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
using namespace std;

kGLShader::kGLShader()
  : m_Handle(0) , m_Linked(false)
{
}

kGLShader::~kGLShader()
{

}

void kGLShader::Release()
{
  if(glIsProgram(m_Handle)) {
    glDeleteProgram(m_Handle);
    m_Handle = 0;
  }
}

bool kGLShader::CompileShaderFromFile(const char *fileName, k3dShader::ShaderType type)
{
  assert(fileName!=NULL);
  if( m_Handle == 0 ) {
    m_Handle = glCreateProgram();
    if( m_Handle == 0) {
      k3dLog::Error("kGLShader : Cannot Create Shader Program !");
      return false;
    }
  }

  ifstream inFile( fileName, ios::in );
  if( !inFile ) {
    return false;
  }

  ostringstream code;
  while( inFile.good() ) {
    int c = inFile.get();
    if( ! inFile.eof() )
      code << (char) c;
  }
  inFile.close();

  bool cRes = CompileShaderFromString(code.str(), type);
  if ( !cRes ) k3dLog::Error("kGLShader Compiled Last Error On File %s", fileName);

  return cRes;
}

bool kGLShader::CompileShaderFromString(const k3dString &source, k3dShader::ShaderType type)
{
  if( m_Handle <= 0 ) {
    m_Handle = glCreateProgram();
    if( m_Handle == 0) {
      k3dLog::Error("kGLShader : Cannot Create Shader Program !");
      return false;
    }
  }

  GLuint shaderHandle = 0;

  switch( type ) {
  case VERTEX:
    shaderHandle = glCreateShader(GL_VERTEX_SHADER);
    break;
  case FRAGMENT:
    shaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
    break;
  case GEOMETRY:
    shaderHandle = glCreateShader(GL_GEOMETRY_SHADER);
    break;
  case TESS_CONTROL:
    shaderHandle = glCreateShader(GL_TESS_CONTROL_SHADER);
    break;
  case TESS_EVALUATION:
    shaderHandle = glCreateShader(GL_TESS_EVALUATION_SHADER);
    break;
  default:
    return false;
  }

  const char * c_code = source.c_str();
  glShaderSource( shaderHandle, 1, &c_code, NULL );
  glCompileShader(shaderHandle );
  int result;
  glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &result );
  if( GL_FALSE == result ) {
    // Compile failed, store log and return false
    int length = 0;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length );
    if( length > 0 ) {
      char * c_log = new char[length];
      int written = 0;
      glGetShaderInfoLog(shaderHandle, length, &written, c_log);
      k3dLog::Error("kGLShader Error Compiled :\n %s", c_log);
      delete [] c_log;
    }

    return false;
  } else {
    glAttachShader(m_Handle, shaderHandle);
    return true;
  }
}

bool kGLShader::Link()
{
  if( m_Linked ) return true;
  if( m_Handle == 0 ) return false;

  glLinkProgram(m_Handle);

  int status = 0;
  glGetProgramiv( m_Handle, GL_LINK_STATUS, &status);
  if( GL_FALSE == status ) {
    int length = 0;
    glGetProgramiv(m_Handle, GL_INFO_LOG_LENGTH, &length );
    if( length > 0 ) {
      char * c_log = new char[length];
      int written = 0;
      glGetProgramInfoLog(m_Handle, length, &written, c_log);
      k3dLog::Error("kGLShader Link Error :\n %s", c_log);
      delete [] c_log;
    }

    return false;
  } else {
    m_Linked = true;
    return m_Linked;
  }
}

bool kGLShader::Validate()
{
  if( !m_Linked ) return false;

  GLint status;
  glValidateProgram( m_Handle );
  glGetProgramiv( m_Handle, GL_VALIDATE_STATUS, &status );

  if( GL_FALSE == status ) {
    int length = 0;
    glGetProgramiv(m_Handle, GL_INFO_LOG_LENGTH, &length );

    if( length > 0 ) {
      char * c_log = new char[length];
      int written = 0;
      glGetProgramInfoLog(m_Handle, length, &written, c_log);
      k3dLog::Error("kGLShader Validate Error :\n %s",c_log);
      delete [] c_log;
    }

    return false;
  } else {
    return true;
  }
}

void kGLShader::Use()
{
  if( m_Handle <= 0 || (! m_Linked) ) return;
  glUseProgram( m_Handle );
}

void kGLShader::BindAttribLocation(uint32 location, const char *name)
{
  glBindAttribLocation(m_Handle, location, name);
}

void kGLShader::BindFragDataLocation(uint32 location, const char *name)
{
  glBindFragDataLocation(m_Handle, location, name);
}

void kGLShader::SetUniform(const char *name, float x, float y, float z)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniform3f(loc, x, y, z);
  }
}

void kGLShader::SetUniform(const char *name, const Vec2f &v)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniform2f(loc, v[0], v[1]);
  }
}

void kGLShader::SetUniform(const char *name, const Vec3f &v)
{
  this->SetUniform(name, v.x, v.y, v.z);
}

void kGLShader::SetUniform(const char *name, const Vec4f &v)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniform4f(loc, v[0], v[1], v[2], v[3]);
  }
}

void kGLShader::SetUniform(const char *name, const Mat4f &m)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)m);
  }
}

void kGLShader::SetUniform(const char *name, float val)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniform1f(loc, val);
  }
}

void kGLShader::SetUniform(const char *name, int val)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniform1i(loc, val);
  }
}

void kGLShader::SetUniform(const char *name, bool val)
{
  int loc = GetUniformLocation(name);
  if( loc >= 0 ) {
    glUniform1i(loc, val);
  }
}

uint32 kGLShader::GetUniformLocation(const char *name)
{
  UniformIter iter = m_CachedUniform.find(k3dString(name));
  if(m_CachedUniform.end() != iter) {
		return iter->second;
  }
	else {
		uint32 loc = glGetUniformLocation(m_Handle, name);
		if (GL_INVALID_INDEX == loc)
		{
			k3dLog::Error("kGLShader::GetUniformLocation Cannot find Uniform %s", name);
			kDebug("Cannot find Uniform %s\n", name);
			return loc;
		}
		m_CachedUniform[k3dString(name)] = loc;
	}

	return m_CachedUniform[k3dString(name)];
}
