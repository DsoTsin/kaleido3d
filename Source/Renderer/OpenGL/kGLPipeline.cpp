#include "kGLPipeline.h"
#include <GL/glew.h>

kGLPipeline::kGLPipeline()
{
  glGenProgramPipelines(1, &m_Pipeline);
}

kGLPipeline::~kGLPipeline()
{
  if(glIsProgramPipeline(m_Pipeline)) {
    glDeleteProgramPipelines(1, &m_Pipeline);
    m_Pipeline = -1;
  }
}

void kGLPipeline::UseShader(k3dShader::ShaderType type, uint32 shaderHandle)
{

  static uint32 shaderBits[] = {
    GL_VERTEX_SHADER_BIT,
    GL_FRAGMENT_SHADER_BIT,
    GL_TESS_CONTROL_SHADER_BIT,
    GL_TESS_EVALUATION_SHADER_BIT,
    GL_GEOMETRY_SHADER_BIT,
    GL_COMPUTE_SHADER_BIT
  };

  glUseProgramStages(m_Pipeline, shaderBits[(int)type], shaderHandle);
}
