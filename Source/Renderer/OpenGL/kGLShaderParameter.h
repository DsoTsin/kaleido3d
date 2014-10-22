#pragma once
#include "kGLShader.h"

class kGLShaderParameter {
public:

  enum {
    SP_Int,
    SP_Float,
    SP_Vec2i,
    SP_Vec2f,
    SP_Vec3i,
    SP_Vec3f,
    SP_Vec4i,
    SP_Vec4f,
    SP_Mat3f,
    SP_Mat4f,
    SP_Tex
  };

  kGLShaderParameter(int type, char* data, int dataSize);
  ~kGLShaderParameter();

  ///
  /// \brief this function is to initialize internal members
  /// \param uniformName
  /// \param shaderHandle
  ///
  void            BindShaderUniformVar(const char* uniformName, uint32 shaderHandle );

  ///
  /// \brief PassShaderUniform called after shader binding
  ///
  void            PassShaderUniform();

  const int &     GetShaderParameterType() const;
  const uint32 &  GetShaderParameterLocation() const;

private:

  char*   m_ParamDataPtr;
  int     m_ParamDataSize;

  int     m_SPType;
  uint32  m_SPLocation;
};
