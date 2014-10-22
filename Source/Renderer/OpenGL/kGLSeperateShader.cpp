#include "kGLSeperateShader.h"
#include <GL/glew.h>

kGLSeperateShader::kGLSeperateShader()
  : m_ShaderHandle(-1)
{

}

kGLSeperateShader::~kGLSeperateShader()
{
  if(glIsProgram(m_ShaderHandle)) {
    glDeleteProgram(m_ShaderHandle);
    m_ShaderHandle = -1;
  }
}
