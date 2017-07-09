#include "Kaleido3D.h"
#include "SceneManager.h"
#include "Camera.h"

#include <Core/LogUtil.h>

#include <fstream>

using namespace std;

namespace k3d {
	//---------------------------------------------------------
	//                    SceneManager
	//---------------------------------------------------------
	SceneManager::SceneManager()
	{
		//  m_SceneRoot = NULL;
		//  m_Octree    = NULL;
	}

	SceneManager::~SceneManager()
	{

	}

	void SceneManager::InitScene()
	{

	}

	void SceneManager::AddSceneObject(SObject::SObjPtr objPtr, const kMath::Vec3f &position)
	{
		K3D_UNUSED(objPtr);
		K3D_UNUSED(position);
		//  objptr->SetPosition(position);
	}

	void SceneManager::FrustumCull(Camera *)
	{

	}

	void SceneManager::UpdateScene()
	{

	}

	bool SceneManager::LoadFromJSON(const char *scene_file)
	{
		ifstream ifs(scene_file);
		if (!ifs.good())
		{
			KLOG(Fatal, "SceneManager", "LoadFromJson Failed. Cannot find %s", scene_file);
			return false;
		}


		return true;
	}

	void SceneManager::SaveToJSON(const char *scene_file)
	{
		K3D_UNUSED(scene_file);
	}
}