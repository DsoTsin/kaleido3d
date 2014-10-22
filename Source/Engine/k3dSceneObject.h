#pragma once

#include <Core/kCoreObject.h>
#include <Math/kGeometry.hpp>
#include "k3dCamera.h"
#include <unordered_map>
#include <Renderer/k3dRenderNode.h>
#include <Core/k3dMaterial.h>

namespace physx {
  class PxRigidBody;
}

enum SOAttrib
{
  SOA_CULLABLE,
  SOA_MOVEABLE,
  SOA_OVERLAY
};


///
/// \brief The kSObject
/// \remarks When the render calls RenderNode, it will cast the render node to drawable object,
///  then excute rendering
///
class kSObject : public k3dRenderNode
{
public:
  typedef std::shared_ptr<kSObject>             SObjPtr;
  typedef std::vector<SObjPtr>                  SOVector;
  typedef SOVector::iterator                    SOVIter;
  typedef std::shared_ptr<class kMeshInstance>  kMIPtr;
  typedef std::shared_ptr<physx::PxRigidBody>   PxRBPtr;
  typedef std::shared_ptr<k3dMaterialBatch>     kMatBatchPtr;

  kSObject();
  virtual ~kSObject();


  const kMath::AABB & GetBoundingBox() const override;
  const kMath::BoundingSphere &getBoundSphere() const override;

  friend class k3dAssetManager;
  friend class k3dSceneManager;

  void          SetNodeType(int type);
  kMatBatchPtr  GetMaterialBatch() const;

protected:

//  void UpdateWorldPosition();

  void UpdateEnable() override;
  void UpdatePosition() override;
  void UpdateTransform() override;

protected:

  PxRBPtr                 m_RigidBodyPtr;
  kMIPtr                  m_MeshPtr;
  kMatBatchPtr            m_MaterialBatchPtr;

//  SOV     m_Children;
  SOAttrib                m_Attrib;
  bool                    m_Visible;
  kMath::Vec3f            m_ObjectPosition;
  kMath::AABB             m_BoundingBox;
  kMath::BoundingSphere   m_BoundingSphere;
  kMath::Mat4f            m_ModelMatrix;
};

