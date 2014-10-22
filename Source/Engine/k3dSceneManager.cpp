#include "k3dSceneManager.h"

#include <fstream>
#include <Core/k3dDbg.h>
#include <Core/k3dLog.h>
#include <json/json.h>

using namespace std;

//---------------------------------------------------------
//                    k3dSceneManager
//---------------------------------------------------------
k3dSceneManager::k3dSceneManager()
{
//  m_SceneRoot = NULL;
//  m_Octree    = NULL;
}

k3dSceneManager::~k3dSceneManager()
{

}

void k3dSceneManager::InitScene()
{

}

void k3dSceneManager::AddSceneObject(kSObject::SObjPtr objptr, const kMath::Vec3f &position)
{
//  objptr->SetPosition(position);
}

void k3dSceneManager::FrustumCull(k3dCamera *)
{

}

void k3dSceneManager::UpdateScene()
{

}

bool k3dSceneManager::LoadFromJSON(const char *scene_file)
{
  ifstream ifs(scene_file);
  if(!ifs.good())
  {
    kDebug("SceneManager::LoadFromJson Failed. Cannot find %s", scene_file);
    return false;
  }

  Json::Reader sceneReader;
  Json::Value sceneRoot;
  if(!sceneReader.parse(ifs, sceneRoot, false))
  {
    kDebug("SceneManager::LoadFromJson Failed. Cannot Parse %s.", scene_file);
    return false;
  }

  return true;
}

void k3dSceneManager::SaveToJSON(const char *scene_file)
{

}
