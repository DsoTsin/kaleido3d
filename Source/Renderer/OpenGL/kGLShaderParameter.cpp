#include "kGLShaderParameter.h"
#include <string>
#include "kGLWrapper.h"
#include <GL/glew.h>
#include <Core/k3dDbg.h>

kGLShaderParameter::kGLShaderParameter(int type, char *data, int dataSize)
  : m_SPType( type )
  , m_ParamDataSize( dataSize )
{
  m_ParamDataPtr = new char[m_ParamDataSize];
  memmove(m_ParamDataPtr, data, m_ParamDataSize);
}

kGLShaderParameter::~kGLShaderParameter()
{
  if(m_ParamDataPtr)
    free(m_ParamDataPtr);
}

void kGLShaderParameter::BindShaderUniformVar(const char *uniformName, uint32 shaderHandle)
{
  m_SPLocation = kGL::GetUniformLocation(uniformName, shaderHandle);
}

void kGLShaderParameter::PassShaderUniform()
{
#define TYPE_OFFSET(ptr, i, type) \
  *((type*)(ptr)+i)

  switch( m_SPType ) {
  case SP_Int:
    glUniform1i(m_SPLocation, TYPE_OFFSET(m_ParamDataPtr, 0, int));
    break;
  case SP_Float:
    glUniform1f(m_SPLocation, TYPE_OFFSET(m_ParamDataPtr, 0, float));
    break;

  case SP_Tex:
    break;

  case SP_Vec2f:
    glUniform2f(m_SPLocation, TYPE_OFFSET(m_ParamDataPtr, 0, float),
                TYPE_OFFSET(m_ParamDataPtr, 1, float) );
    break;

  case SP_Vec3f:
    glUniform3f(m_SPLocation, TYPE_OFFSET(m_ParamDataPtr, 0, float),
                TYPE_OFFSET(m_ParamDataPtr, 1, float),
                TYPE_OFFSET(m_ParamDataPtr, 2, float));
    break;

  case SP_Vec4f:

    glUniform4f(m_SPLocation, TYPE_OFFSET(m_ParamDataPtr, 0, float),
                TYPE_OFFSET(m_ParamDataPtr, 1, float),
                TYPE_OFFSET(m_ParamDataPtr, 2, float),
                TYPE_OFFSET(m_ParamDataPtr, 3, float) );
    break;

  case SP_Mat3f:
    glUniformMatrix3fv(m_SPLocation, 1, GL_FALSE, (float*)m_ParamDataPtr);
    break;

  case SP_Mat4f:
    glUniformMatrix4fv(m_SPLocation, 1, GL_FALSE, (float*)m_ParamDataPtr);
    break;

  default:
    kDebug("Not supported parameter type!\n");
    break;
  }
}

const int &kGLShaderParameter::GetShaderParameterType() const
{
  return m_SPType;
}

const uint32 &kGLShaderParameter::GetShaderParameterLocation() const
{
  return m_SPLocation;
}
