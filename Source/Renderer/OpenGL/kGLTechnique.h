#pragma once
#include "kGLShader.h"
#include <Renderer/k3dTechnique.h>

class kGLTechnique : public k3dTechnique {
public:

  kGLTechnique();
  ~kGLTechnique();

  void  PrintAllProgramNames() override;
  int   GetProgramCount() override;
  void  ParseFXFromFile(const char *name) override;
  void  ParseFXFromMemory(const k3dString & source) override;

  void  EnableTechnique(const char *techName) override;
  void  EnableTechnique(uint32 technique) override;

  uint32 GetProgram(const char *techName) const;

private:

  int m_EffectHandle;
  typedef std::unordered_map<k3dString, uint32> MapTechs;
  typedef MapTechs::iterator                    MapTechsIter;
  typedef MapTechs::const_iterator              MapTechsCIter;
  MapTechs m_TechContainer;
};
