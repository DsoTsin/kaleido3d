#pragma once

#include <Engine/k3dSceneObject.h>

namespace physx {
 class PxRigidDynamic;
}

class kMoveable : public kSObject {
public:
  kMoveable();
  ~kMoveable();

  void InitWithMesh(std::shared_ptr<class k3dMesh> &meshInstance);
  void UpdateTrasnform();

private:

  std::shared_ptr<physx::PxRigidDynamic> m_RigidBody;
};
