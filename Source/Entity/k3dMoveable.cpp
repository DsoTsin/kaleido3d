#include "k3dMoveable.h"
#include <Core/k3dMesh.h>
#include <Physics/PxSupport.h>

using namespace kMath;
using namespace physx;

kMoveable::kMoveable()
{
}

kMoveable::~kMoveable()
{
}

void kMoveable::InitWithMesh(std::shared_ptr<k3dMesh> &meshInstance)
{

}

void kMoveable::UpdateTrasnform()
{
  PxTransform psWorld = m_RigidBody->getGlobalPose();
  PxMat44 psMat(psWorld);
  ConvertPxMat44toMat4f(psMat, m_ModelMatrix);
}
