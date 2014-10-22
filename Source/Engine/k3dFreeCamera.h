#pragma once
#include "k3dCamera.h"

class kFreeCamera : public k3dCamera
{
public:

  kFreeCamera();
  ~kFreeCamera();

  void Update();

  void Walk(const float dt);
  void Strafe(const float dt);
  void Lift(const float dt);

  void SetTranslation(const kMath::Vec3f& t);
  kMath::Vec3f GetTranslation() const;

  void SetSpeed(const float speed);
  const float GetSpeed() const;

protected:

  float         m_CameraSpeed;
  kMath::Vec3f  m_Translation;
};
