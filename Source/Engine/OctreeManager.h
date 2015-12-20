#pragma once
#include <Math/kGeometry.hpp>

#include "SceneObject.h"
#include "SceneManager.h"

namespace k3d {

	//! \class	OctreeSceneManager
	//! \brief	this part is referenced to 
	//! 		the implementation of SceneManager
	//! 		of KlayGE<http://klayge.org>
	class OctreeSceneManager : public SceneManager {
	public:
		OctreeSceneManager();

		KOBJECT_PROPERTY(MaxTreeDepth, uint32);

		//! \fn	bool OctreeSceneManager::AABBVisible(kMath::AABB const & aabb);
		//! \brief	Test AABB
		//! \param[in]	aabb	The AABB
		//! \return	true if it succeeds, false if it fails.
		bool AABBVisible(kMath::AABB const & aabb);

		//! \fn	bool OctreeSceneManager::SphereVisible(kMath::BoundingSphere const & sphere);
		//! \brief	Sphere visible.
		//! \param[in]	sphere	The sphere.
		//! \return	true if it succeeds, false if it fails.
		bool SphereVisible(kMath::BoundingSphere const & sphere);

	private:
		void ClipScene();
		void ClearObject();

		void AddSceneObject(SObject::SObjPtr const & obj);
		void DeleteSceneObject();

		void SplitNode(int index, uint32 curNodeDepth);
		void NodeVisible(int index);
		void MarkNodeObjs(int index, bool force);

		bool BoundVisible(int index, kMath::AABB const & aabb) const;

	private:
		K3D_DISCOPY(OctreeSceneManager)

	private:

		struct kOctreeNode
		{
			kMath::AABB           AABB;
			int                   FirstChildIndex;
			BoundType             Visible;
			SObject::SOVector     ObjPtrs;
		};

		typedef std::vector<kOctreeNode> OctVector;

		OctVector m_Octree;
		uint32    m_MaxTreeDepth;
		bool      m_RebuildTree;
	};
}