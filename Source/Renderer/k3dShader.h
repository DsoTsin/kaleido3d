#pragma once
#include <Math/kMath.hpp>
#include <sstream>

using kMath::Vec2i;
using kMath::Vec2f;
using kMath::Vec3f;
using kMath::Vec4f;
using kMath::Mat4f;

///
/// \brief The ShaderOptions class
///
class ShaderOptions
{
public:
  static ShaderOptions 	CreateVSOptions();
  static ShaderOptions 	CreateFSOptions(int w, int );

  template<typename T>
  void          AddDefine(	const std::string& name, const T& value) {
    std::stringstream option;
    option << "#define " << name << " " << value;
    options.push_back(option.str());
  }

  void          AddResolution(const std::string& name, int resX, int resY);
  void          AddConst(const std::string& name, int value);
  void          AddConst(const std::string& name, float value);
  void          AddConst(const std::string& name, const Vec2i& value);
  void          Include(const std::string& _string);
  std::string	ToString() const;
  std::string	Append(const std::string& source) const;

private:
  std::vector<std::string> options;
};

class k3dShader
{
public:
  enum ShaderType
  {
    VERTEX, FRAGMENT,
    TESS_CONTROL, TESS_EVALUATION, GEOMETRY,
    COMPUTE
  };

  k3dShader() {}
  virtual ~k3dShader() {}

  virtual void   Release() = 0;

  virtual bool   CompileShaderFromFile( const char * fileName, ShaderType type ) = 0;
  virtual bool   CompileShaderFromString( const k3dString & source, ShaderType type ) = 0;
  virtual bool   Link() = 0;
  virtual void   Use() = 0;

  virtual void   BindAttribLocation( uint32 location, const char * name) = 0;
  virtual void   BindFragDataLocation( uint32 location, const char * name ) = 0;

  virtual void   SetUniform( const char *name, float x, float y, float z) = 0;
  virtual void   SetUniform( const char *name, const Vec2f & v) = 0;
  virtual void   SetUniform( const char *name, const Vec3f & v) = 0;
  virtual void   SetUniform( const char *name, const Vec4f & v) = 0;
  virtual void   SetUniform( const char *name, const Mat4f & m) = 0;
  virtual void   SetUniform( const char *name, float val ) = 0;
  virtual void   SetUniform( const char *name, int val ) = 0;
  virtual void   SetUniform( const char *name, bool val ) = 0;

protected:
  typedef std::unordered_map<k3dString, uint32> MapCachedUniform;
};
