#ifndef __kGLShader_h__
#define __kGLShader_h__
#pragma once

#include <Renderer/k3dShader.h>

class kGLShader : public k3dShader/*, public kCoreObject*/ {
public:
//  KOBJECT_PTR_CAST(kGLShader)

  kGLShader();
  ~kGLShader();

  virtual void Release() override;

//-------------------------------------------------------------------------------
  bool   CompileShaderFromFile( const char * fileName, ShaderType type ) override;
  bool   CompileShaderFromString( const k3dString & source, ShaderType type ) override;
  bool   Link() override;
  bool   Validate();
  void   Use() override;

  void   BindAttribLocation( uint32 location, const char * name);
  void   BindFragDataLocation( uint32 location, const char * name );

  void   SetUniform( const char *name, float x, float y, float z);
  void   SetUniform( const char *name, const Vec2f & v);
  void   SetUniform( const char *name, const Vec3f & v);
  void   SetUniform( const char *name, const Vec4f & v);
  void   SetUniform( const char *name, const Mat4f & m);
  void   SetUniform( const char *name, float val );
  void   SetUniform( const char *name, int val );
  void   SetUniform( const char *name, bool val );
//--------------------------------------------------------------------------------

  typedef MapCachedUniform::iterator UniformIter;

private:
  uint32  GetUniformLocation(const char *name);

  uint32  m_Handle;
  bool    m_Linked;
  MapCachedUniform m_CachedUniform;
};

#endif
