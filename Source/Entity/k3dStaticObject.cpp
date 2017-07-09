#include "k3dStaticObject.h"
#include <Core/k3dMesh.h>
#include <Physics/kPsEngine.h>
#include <PxPhysicsAPI.h>
#include <PxToolkit.h>
#include <Math/kMath.hpp>
#include <Math/NvPxMathAdapter.hpp>

using namespace physx;

k3dStaticObject::k3dStaticObject()
{

}

k3dStaticObject::~k3dStaticObject()
{

}

bool k3dStaticObject::InitWithStaticMesh(std::shared_ptr<k3dMesh> &meshInstance)
{
  PxConvexMesh *convexMesh = kPsEngine::createConvexMesh(
        (const PxVec3*)meshInstance->GetVertexBuffer(),
        (const PxU32)meshInstance->GetVertexNum());

  if( convexMesh == nullptr )
    return false;

  PxShape *rigidShape = m_RigidStatic->createShape(PxConvexMeshGeometry(convexMesh), *(kPsEngine::Get().DefaultMaterialPtr()) );
  m_RigidStatic->attachShape(*rigidShape);

  return true;
}
