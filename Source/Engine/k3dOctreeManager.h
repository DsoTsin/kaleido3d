#pragma once
#include <Config/Prerequisities.h>
#include <Math/kGeometry.hpp>

#include "k3dSceneObject.h"
#include "k3dSceneManager.h"
//! \class	kOctreeSceneManager
//! \brief	this part is referenced to 
//! 				the implementation of SceneManager
//! 				of KlayGE<http://klayge.org>
class kOctreeSceneManager : public k3dSceneManager
{
public:
  kOctreeSceneManager();

  KOBJECT_PROPERTY(MaxTreeDepth, uint32)

  //! \fn	bool kOctreeSceneManager::AABBVisible(kMath::AABB const & aabb);
  //! \brief	Test AABB
  //! \param[in]	aabb	The AABB
  //! \return	true if it succeeds, false if it fails.
  bool AABBVisible(kMath::AABB const & aabb);

  //! \fn	bool kOctreeSceneManager::SphereVisible(kMath::BoundingSphere const & sphere);
  //! \brief	Sphere visible.
  //! \param[in]	sphere	The sphere.
  //! \return	true if it succeeds, false if it fails.
  bool SphereVisible(kMath::BoundingSphere const & sphere);

private:
  void ClipScene();
  void ClearObject();

  void AddSceneObject(kSObject::SObjPtr const & obj);
  void DeleteSceneObject();

  void SplitNode(int index, uint32 curNodeDepth);
  void NodeVisible(int index);
  void MarkNodeObjs(int index, bool force);

  bool BoundVisible(int index, kMath::AABB const & aabb) const;

private:
  K3DDISCOPY(kOctreeSceneManager)

private:

  struct kOctreeNode
  {
    kMath::AABB           AABB;
    int                   FirstChildIndex;
    BoundType             Visible;
    kSObject::SOVector    ObjPtrs;
  };

  typedef std::vector<kOctreeNode> OctVector;

  OctVector m_Octree;
  uint32    m_MaxTreeDepth;
  bool      m_RebuildTree;
};
