#include "kGLSamplerObj.h"
#include <GL/glew.h>

kGLSamplerObj::kGLSamplerObj()
  : m_SampleHandle(-1)
{
  glGenSamplers(1, &m_SampleHandle);
}

kGLSamplerObj::~kGLSamplerObj()
{
  if(glIsSampler(m_SampleHandle))
  {
    glDeleteSamplers(1, &m_SampleHandle);
    m_SampleHandle = -1;
  }
}

void kGLSamplerObj::SetFilter(const kGLSamplerParamDesc &samplerDesc)
{
  glSamplerParameteri(m_SampleHandle, GL_TEXTURE_MIN_FILTER, samplerDesc.MinFilter);
  glSamplerParameteri(m_SampleHandle, GL_TEXTURE_MAG_FILTER, samplerDesc.MagFilter);
}

void kGLSamplerObj::BindTexture(uint32 texId, uint32 target, uint32 texUnit)
{
  glBindMultiTextureEXT(texUnit, target, texId);
}

void kGLSamplerObj::BindTexUnit(uint32 texUnit)
{
  glBindSampler(texUnit, m_SampleHandle);
}
