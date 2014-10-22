#pragma once

struct kGLSamplerParamDesc {
  uint32 MinFilter;
  uint32 MagFilter;
};

class kGLSamplerObj {
public:
  kGLSamplerObj();
  ~kGLSamplerObj();

  void SetFilter(const kGLSamplerParamDesc & samplerDesc);

  ///
  /// \brief BindTexUnit
  /// \param texUnit [from 0 to max tex unit]
  ///
  void BindTexUnit(uint32 texUnit);

  ///
  /// \brief BindTexture
  /// \param texId
  /// \param tex target [GL_TEXTURE_2D...]
  /// \param texUnit [0 to Max Texture Unit]
  ///
  static void BindTexture(uint32 texId, uint32 target, uint32 texUnit);

private:

  uint32 m_SampleHandle;
};
