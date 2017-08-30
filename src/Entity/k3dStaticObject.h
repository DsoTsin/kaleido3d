#pragma once

namespace physx {
 class PxRigidStatic;
}

class k3dStaticObject {
public:
  k3dStaticObject();
  virtual ~k3dStaticObject();

  bool InitWithStaticMesh(std::shared_ptr<class k3dMesh> &meshInstance);

private:
  std::shared_ptr<physx::PxRigidStatic>  m_RigidStatic;

};
