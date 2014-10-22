#pragma once

#include <Renderer/k3dShader.h>

class kGLSeperateShader {
public:
  kGLSeperateShader();
  ~kGLSeperateShader();

private:
  uint32 m_ShaderHandle;
};
