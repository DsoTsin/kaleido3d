#pragma once
#include "k3dLight.h"


class k3dLightProbe : public k3dLight {
public:
  k3dLightProbe();
  k3dLightProbe( const kMath::Vec4f & color, const kMath::Vec3f & radius );
  ~k3dLightProbe();

private:

  kMath::Vec3f m_Radius;
  kMath::Vec3f m_SHCoeff[9];

};
