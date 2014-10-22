#pragma once

#include <Renderer/k3dShader.h>

class kGLPipeline {
public:
  kGLPipeline();
  ~kGLPipeline();

  void UseShader(k3dShader::ShaderType type, uint32 shaderHandle);

private:
  uint32 m_Pipeline;
};
