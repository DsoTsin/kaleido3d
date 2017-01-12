#pragma once

#include <Engine/k3dSceneObject.h>

namespace physx {
  class PxRigidStatic;
}

class k3dTerrian : public kSObject
{
public:

  k3dTerrian();
  ~k3dTerrian();

  typedef std::shared_ptr<physx::PxRigidStatic> RigidStaticPtr;

  RigidStaticPtr GetRigidPtr() const;

private:

   RigidStaticPtr m_RigidBody;
};
