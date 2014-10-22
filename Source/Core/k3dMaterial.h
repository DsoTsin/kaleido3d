
#pragma once
#include <Config/Prerequisities.h>
#include <Math/kMath.hpp>

struct BaseMaterialParam {
  uint32 MatNodeID;

  kMath::Vec3f DiffuseColor;
  kMath::Vec3f SpecularColor;
  kMath::Vec3f AmbientColor;
  kMath::Vec3f EmissiveColor;

  k3dString DiffuseMap;
  k3dString SpecularMap;
  k3dString AmbientMap;
  k3dString NormalMap;
  k3dString EmissiveMap;
  k3dString TransparencyMap;
  k3dString DisplaceMap;

  float Shininess;
  float Transparency;

  BaseMaterialParam() {
    MatNodeID = 0;
    Shininess = 0.0f;
    Transparency = 0.0f;
  }
};

class BaseMaterialShader {
public:
  static const uint32 Is_Emissive        = 1;
  static const uint32 Is_Semitransparent = (1<<1);
  static const uint32 Has_DiffuseMap     = (1<<2);
  static const uint32 Has_SpecularMap    = (1<<3);
  static const uint32 Has_AmbientMap     = (1<<4);
  static const uint32 Has_NormalMap      = (1<<5);
  static const uint32 Has_EmissiveMap    = (1<<6);
  static const uint32 Has_TransMap       = (1<<7);
  static const uint32 Has_DisplacementMap= (1<<8);

  BaseMaterialShader();
  bool GenerateShaderFromMaterial( const BaseMaterialParam & params );
  inline uint32 GetMaterialAttrib() const;

  friend class k3dMaterial;

private:
  void    GetMaterialAttrib( const BaseMaterialParam & params );

  uint32    m_MatAttrib; // record material params
  std::shared_ptr<class k3dShader> m_MatShaderInstance;
};

class k3dMaterial {
public:
  enum {
    Material_Opaque,
    Material_Translucent,
    Material_Mask
  };

  k3dMaterial();

  void  SetMaterialType(int materialType);
  int   GetMaterialType() const;

  void  InitShaderVariableInternalBinding();

  bool IsReflective() const;
  bool IsEmissive() const;

  typedef std::shared_ptr<class k3dTexture>       kTexPtr;
  typedef std::map<uint32, BaseMaterialParam>     MatParams;
  typedef std::map<uint32, BaseMaterialShader*>   MatShaders;

private:

  uint32  m_ShaderNativeHandle;

  char    m_MaterialType;
  bool    m_IsShaderLocationBinded;
};

class k3dMaterialBatch {
public:

  typedef std::shared_ptr<k3dMaterial> SharedMaterialPtr;
  typedef std::vector<SharedMaterialPtr> MaterialPtrVector;

  int                 GetBatchCount() const;
  SharedMaterialPtr   GetMaterialPtr(int i) const;

private:

  MaterialPtrVector   m_MaterialPtrs;
};
