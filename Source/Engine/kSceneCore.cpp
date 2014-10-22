#include "k3dSceneObject.h"
#include <json/json.h>
#include <Physics/PxSupport.h>

kSObject::kSObject()
  : k3dRenderNode( Node_Object )
{
}

kSObject::~kSObject()
{
}

const kMath::AABB &kSObject::GetBoundingBox() const
{
  return m_BoundingBox;
}

const kMath::BoundingSphere &kSObject::getBoundSphere() const
{
  return m_BoundingSphere;
}

void kSObject::SetNodeType(int type)
{
  m_NodeType = type;
}

kSObject::kMatBatchPtr kSObject::GetMaterialBatch() const
{
  return m_MaterialBatchPtr;
}

void kSObject::UpdateEnable()
{

}

void kSObject::UpdatePosition()
{

}

void kSObject::UpdateTransform()
{
  PxTransform psWorld = m_RigidBodyPtr->getGlobalPose();
  PxMat44 psMat(psWorld);
  ConvertPxMat44toMat4f(psMat, m_ModelMatrix);
}
