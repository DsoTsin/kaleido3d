#include "Kaleido3D.h"
#include "OctreeManager.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <algorithm>
#include <assert.h>

using namespace kMath;

namespace k3d {

	OctreeSceneManager::OctreeSceneManager()
	{
		m_MaxTreeDepth = 4;
		m_RebuildTree = false;
	}

	void OctreeSceneManager::SetMaxTreeDepth(const uint32 &_MaxTreeDepth)
	{
		m_MaxTreeDepth = std::min<uint32>(_MaxTreeDepth, 16u);
	}

	uint32 OctreeSceneManager::GetMaxTreeDepth() const
	{
		return m_MaxTreeDepth;
	}

	void OctreeSceneManager::ClipScene()
	{
		if (m_RebuildTree)
		{
			m_Octree.resize(1);
			AABB bb_root(Vec3f(0.f, 0.f, 0.f), Vec3f(0.f, 0.f, 0.f));
			m_Octree[0].FirstChildIndex = -1;
			m_Octree[0].Visible = BO_NO;

			for (SObject::SObjPtr &obj : m_SceneObjs)
			{
				uint32 type = obj->GetType();
				if (type == /*RenderNode::Node_Object*/-1)
				{
					bb_root |= obj->GetBoundingBox();
					m_Octree[0].ObjPtrs.push_back(obj);
				}
			}

			Vec3f center = bb_root.GetCenter();
			Vec3f extent = bb_root.GetHalf();
			float longest_dim = std::max(extent.z, std::max(extent.x, extent.y));

			Vec3f new_extent = Vec3f(longest_dim, longest_dim, longest_dim);

			m_Octree[0].AABB = AABB(center - new_extent, center + new_extent);

			this->SplitNode(0, 1);
			//    this->DivideNode(0, 1);

			m_RebuildTree = false;
		}

		if (!m_Octree.empty())
		{
			this->NodeVisible(0);
		}


		//  App3DFramework& app = Context::Instance().AppInstance();
		//  Camera& camera = app.ActiveCamera();

		//  float4x4 view_proj = camera.ViewProjMatrix();;
		//  DeferredRenderingLayerPtr const & drl = Context::Instance().DeferredRenderingLayerInstance();
		//  if (drl)
		//  {
		//    float4x4 proj = camera.ProjMatrix();
		//    int32_t cas_index = drl->CurrCascadeIndex();
		//    if (cas_index >= 0)
		//    {
		//      view_proj *= drl->GetCascadedShadowLayer()->CascadeCropMatrix(cas_index);
		//    }
		//  }

		//  if (camera.OmniDirectionalMode())
		//  {
		//    KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, scene_objs_)
		//    {
		//      SceneObjectPtr const & obj = soaabb->so;
		//      soaabb->visible = (!(obj->Attrib() & SceneObject::SOA_Overlay) && obj->Visible());

		//      if (obj->Attrib() & SceneObject::SOA_Cullable)
		//      {
		//        AABBox aabb_ws;
		//        if (obj->Attrib() & SceneObject::SOA_Moveable)
		//        {
		//          AABBox const & aabb = obj->PosBound();
		//          float4x4 const & mat = obj->ModelMatrix();

		//          aabb_ws = MathLib::transform_aabb(aabb, mat);
		//        }
		//        else
		//        {
		//          aabb_ws = *soaabb->aabb_ws;
		//        }

		//        soaabb->visible &= (MathLib::perspective_area(camera.EyePos(), view_proj,
		//                                                      aabb_ws) > small_obj_threshold_);
		//      }
		//    }
		//  }
		//  else
		//  {
		//    if (!octree_.empty())
		//    {
		//      this->MarkNodeObjs(0, false);
		//    }

		//    KLAYGE_FOREACH(SceneObjAABBPtrType const & soaabb, scene_objs_)
		//    {
		//      SceneObjectPtr const & obj = soaabb->so;
		//      if (obj->Visible())
		//      {
		//        uint32_t const attr = obj->Attrib();
		//        if (!(attr & SceneObject::SOA_Overlay))
		//        {
		//          if (!(attr & SceneObject::SOA_Cullable))
		//          {
		//            soaabb->visible = true;
		//          }
		//          else if (attr & SceneObject::SOA_Moveable)
		//          {
		//            AABBox const & aabb = obj->PosBound();
		//            float4x4 const & mat = obj->ModelMatrix();
		//            soaabb->visible = this->AABBVisible(MathLib::transform_aabb(aabb, mat));
		//          }
		//        }
		//      }
		//    }
		  //  }
	}

	void OctreeSceneManager::AddSceneObject(const SObject::SObjPtr & obj)
	{
		K3D_UNUSED(obj);
		//  AABBoxPtr aabb_ws;
		//  uint32_t const attr = obj->Attrib();
		//  if ((attr & SceneObject::SOA_Cullable)
		//      && !(attr & SceneObject::SOA_Overlay)
		//      && !(attr & SceneObject::SOA_Moveable))
		//  {
		//      AABBox const & aabb = obj->PosBound();
		//      float4x4 const & mat = obj->ModelMatrix();
		//      aabb_ws = MakeSharedPtr<AABBox>(MathLib::transform_aabb(aabb, mat));
		//  }

		//  scene_objs_.push_back(MakeSharedPtr<SceneObjAABB>(obj, aabb_ws, false));

		//  this->OnAddSceneObject(obj);
	}

	void OctreeSceneManager::DeleteSceneObject()
	{

	}

	void OctreeSceneManager::SplitNode(int index, uint32 curNodeDepth)
	{
		if (m_Octree[index].ObjPtrs.size() > 1)
		{
			size_t const this_size = m_Octree.size();
			AABB const parent_bb = m_Octree[index].AABB;
			Vec3f const parent_center = parent_bb.GetCenter();
			m_Octree[index].FirstChildIndex = static_cast<int>(this_size);
			m_Octree[index].Visible = BO_NO;

			m_Octree.resize(this_size + 8);
			for (SObject::SObjPtr const & soaabb : m_Octree[index].ObjPtrs)
			{
				AABB const & aabb = soaabb->GetBoundingBox();
				int mark[6];
				mark[0] = aabb.GetMinCorner()[0] >= parent_center[0] ? 1 : 0;
				mark[1] = aabb.GetMinCorner()[1] >= parent_center[1] ? 2 : 0;
				mark[2] = aabb.GetMinCorner()[2] >= parent_center[2] ? 4 : 0;
				mark[3] = aabb.GetMaxCorner()[0] >= parent_center[0] ? 1 : 0;
				mark[4] = aabb.GetMaxCorner()[1] >= parent_center[1] ? 2 : 0;
				mark[5] = aabb.GetMaxCorner()[2] >= parent_center[2] ? 4 : 0;
				for (int j = 0; j < 8; ++j)
				{
					if (j == ((j & 1) ? mark[3] : mark[0])
						+ ((j & 2) ? mark[4] : mark[1])
						+ ((j & 4) ? mark[5] : mark[2]))
					{
						m_Octree[this_size + j].ObjPtrs.push_back(soaabb);
					}
				}
			}

			for (size_t j = 0; j < 8; ++j)
			{
				kOctreeNode & new_node = m_Octree[this_size + j];
				new_node.FirstChildIndex = -1;
				new_node.AABB = AABB(Vec3f((j & 1) ? parent_center[0] : parent_bb.GetMinCorner()[0],
					(j & 2) ? parent_center[1] : parent_bb.GetMinCorner()[1],
					(j & 4) ? parent_center[2] : parent_bb.GetMinCorner()[2]),
					Vec3f((j & 1) ? parent_bb.GetMaxCorner()[0] : parent_center[0],
					(j & 2) ? parent_bb.GetMaxCorner()[1] : parent_center[1],
					(j & 4) ? parent_bb.GetMaxCorner()[2] : parent_center[2]));

				if (curNodeDepth < m_MaxTreeDepth)
				{
					this->SplitNode((int)(this_size + j), curNodeDepth + 1);
				}
			}

			SObject::SOVector empty;
			m_Octree[index].ObjPtrs.swap(empty);
		}
	}

	void OctreeSceneManager::NodeVisible(int index)
	{
		//  bool visible = true;

		kOctreeNode& node = m_Octree[index];
		node.Visible = m_CameraPtr->IntersectBox(node.AABB);

		// assure the node is in visible distance then
		// check the visiblity of the child node
		if (BO_PARTIAL == node.Visible)
		{
			if (node.FirstChildIndex != -1)
			{
				for (int i = 0; i < 8; ++i)
				{
					this->NodeVisible(node.FirstChildIndex + i);
				}
			}
		}
	}

	void OctreeSceneManager::MarkNodeObjs(int index, bool force)
	{
		assert(index < m_Octree.size());
		kOctreeNode const & node = m_Octree[index];
		if ((node.Visible != BO_NO) || force)
		{
			for (SObject::SObjPtr const & obj : node.ObjPtrs)
			{
				if (!obj->IsVisible() /*&& soaabb->so->Visible()*/)
				{
					//        if (MathLib::perspective_area(camera.EyePos(), view_proj,
					//                                      *soaabb->aabb_ws) > small_obj_threshold_)
					//        {
					//          BoundType const bo = m_CameraPtr->IntersectBox( obj->GetBoundingBox() );
					//          obj->SetVisiblity(bo != BO_NO);
					//        }
					//        else
					//        {
					//          obj->SetVisiblity( false );
					//        }
					BoundType const bo = m_CameraPtr->IntersectBox(obj->GetBoundingBox());
					obj->SetVisible(bo != BO_NO);
				}
			}

			if (node.FirstChildIndex != -1)
			{
				for (int i = 0; i < 8; ++i)
				{
					this->MarkNodeObjs(node.FirstChildIndex + i, (BO_YES == node.Visible) || force);
				}
			}
		}

	}

	bool OctreeSceneManager::BoundVisible(int index, const AABB &aabb) const
	{
		K3D_UNUSED(index);
		K3D_UNUSED(aabb);
		return true;
	}

}