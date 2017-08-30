#pragma once
#include <Math/kGeometry.hpp>
#include <KTL/DynArray.hpp>
#include <KTL/SharedPtr.hpp>
#include "Camera.h"
//#include <unordered_map>

namespace physx {
  class PxRigidBody;
}

namespace k3d {

	enum SOAttrib
	{
		SOA_CULLABLE,
		SOA_MOVEABLE,
		SOA_OVERLAY
	};

	///
	/// \brief The SObject
	/// \remarks When the render calls RenderNode, it will cast the render node to drawable object,
	///  then excute rendering
	///
	class SObject
	{
	public:
		typedef SharedPtr<SObject>             SObjPtr;
		typedef DynArray<SObjPtr>                  SOVector;
		typedef SOVector::iterator                    SOVIter;

		SObject() = default;
		virtual ~SObject();


		friend class AssetManager;
		friend class SceneManager;

		void          SetNodeType(int type);
		virtual int	  GetType() = 0;
						
		const kMath::AABB & GetBoundingBox() const;
		bool IsVisible();
		void SetVisible(bool visible);

	protected:

		//  void UpdateWorldPosition();

		  /*void UpdateEnable() override;
		  void UpdatePosition() override;
		  void UpdateTransform() override;*/

	protected:

		//  SOV     m_Children;
		SOAttrib                m_Attrib;
		bool                    m_Visible;
		kMath::Vec3f            m_ObjectPosition;
		kMath::AABB             m_BoundingBox;
		kMath::BoundingSphere   m_BoundingSphere;
		kMath::Mat4f            m_ModelMatrix;
	};

}