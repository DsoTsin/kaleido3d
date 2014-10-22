#include "kGLTechnique.h"
#include <GL/glfx.h>
#include <GL/glew.h>
#include <Core/k3dDbg.h>
#include <Core/k3dLog.h>
#include <Core/k3dFile.h>

kGLTechnique::kGLTechnique()
  : m_EffectHandle(-1)
{
}

kGLTechnique::~kGLTechnique()
{
  if(m_EffectHandle != -1)  {
    glfxDeleteEffect(m_EffectHandle);
    m_EffectHandle = -1;
  }
}

void kGLTechnique::PrintAllProgramNames()
{
  if( !m_TechContainer.empty() ) {
    for( auto p : m_TechContainer )
    {
      kDebug("TechName : %s .\n", p.first.c_str() );
    }
  }
}

int kGLTechnique::GetProgramCount()
{
  return glfxGetProgramCount(m_EffectHandle);
}

void kGLTechnique::ParseFXFromFile(const char *name)
{
  m_EffectHandle = glfxGenEffect();
  assert(m_EffectHandle!=-1&&"glfx effect not init");
  k3dFile shaderFile(name);
  if(!shaderFile.Open(IORead)) {
    k3dLog::Error("kGLTechnique::ParseFXFromFile Error.");
    return ;
  }

  int shaderLength = (int)shaderFile.GetSize();
  char *shaderSrc = new char [shaderLength+1];
  shaderFile.Read(shaderSrc, shaderLength);
  shaderSrc[shaderLength] = 0;

  if(!glfxParseEffectFromMemory(m_EffectHandle, shaderSrc)) {
    const char *fxLog = glfxGetEffectLog(m_EffectHandle);
    k3dLog::Message("kGLTechnique : \n %s", fxLog);
    kDebug("Technique Failed to Parse. %s\n", fxLog);
    return ;
  }

  int prog_count = glfxGetProgramCount(m_EffectHandle);
  const char *prog_name = nullptr;
  for(int i=0; i<prog_count; i++)
  {
    prog_name = glfxGetProgramName(m_EffectHandle, i);
    int prog_handle = glfxCompileProgram(m_EffectHandle, prog_name);
    if(-1 == prog_handle)
    {
      kDebug("FX Error: %s\n", glfxGetEffectLog(m_EffectHandle));
      return ;
    }
    m_TechContainer[prog_name] = prog_handle;
    k3dLog::Message("technique %s compilation passed .", prog_name);
  }
  k3dLog::Message("kGLTechnique (%s) Passed.", name);
}

void kGLTechnique::ParseFXFromMemory(const k3dString &source)
{
  K3D_UNUSED(source);
}

void kGLTechnique::EnableTechnique(const char *techName)
{
  MapTechsIter iter = m_TechContainer.find(techName);
  if(iter==m_TechContainer.end()) {
    kDebug("Cannot find technique %s .", techName);
    return;
  }
  glUseProgram(iter->second);
}

void kGLTechnique::EnableTechnique(uint32 technique)
{
  glUseProgram(technique);
}

uint32 kGLTechnique::GetProgram(const char *techName) const
{
  MapTechsCIter iter = m_TechContainer.find(techName);
  if(iter==m_TechContainer.end()) {
    kDebug("Cannot find technique %s .", techName);
    return -1;
  }
  else
    return iter->second;
}
