#pragma once
#include <Engine/k3dSceneObject.h>
#include <Math/kMath.hpp>
#include <Renderer/k3dTexture.h>
#include <json/json.h>

class k3dLight : public k3dRenderNode {
public:
  k3dLight();
  virtual ~k3dLight();

  bool LoadJson(Json::Value & lightNode);
  bool ObjectInVolume(kSObject* obj);
  bool NodeInVolume(kMath::Vec3f center, float size);


protected:

  kMath::Vec4f  m_Color;

  bool volume_test;
  kMath::Vec3f VolumeCorners[8];
  kMath::Vec3f VolumeNormal[6];
  float VolumeD[6];

  float         m_Attenuation;
  float         m_Multiplier;
  float         m_DiffuseScale;
  float         m_SpecularPower;
  bool          m_CastShadow;
  float         m_ShadowBias;
  float         m_ShadowDist;
  float         m_RenderDist;
  float         m_VisibleDist; //visible distance

  kTexPtr m_ColorTex;
  kTexPtr m_DepthTex;
};
