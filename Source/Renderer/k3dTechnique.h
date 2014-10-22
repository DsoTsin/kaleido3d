#pragma once

class k3dTechnique {
public:
  k3dTechnique();
  virtual ~k3dTechnique();

  virtual void  PrintAllProgramNames() = 0;
  virtual int   GetProgramCount() = 0;
  virtual void  ParseFXFromFile(const char *name) = 0;
  virtual void  ParseFXFromMemory(const k3dString & source) = 0;

  virtual void  EnableTechnique(const char *techName) = 0;
  virtual void  EnableTechnique(uint32 technique) = 0;
};

typedef std::shared_ptr<k3dTechnique> SharedTechniquePtr;
