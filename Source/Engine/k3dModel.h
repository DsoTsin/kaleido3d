#pragma once
#include <Config/Prerequisities.h>

class k3dModel {
public:
  k3dModel();

  typedef std::shared_ptr<class k3dMesh>      MeshPtr;
  typedef std::vector<MeshPtr>                MeshPtrArray;
  typedef std::shared_ptr<class k3dMaterial>  MatPtr;
  typedef std::vector<MatPtr>                 MatPtrArray;

private:
  K3DDISCOPY(k3dModel)
private:

  MeshPtrArray  m_MeshInstances;
  MatPtrArray   m_MeshMaterials;
};
