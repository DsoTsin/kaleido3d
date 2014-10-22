#pragma once
#include <KTL/Singleton.h>
#include "k3dSceneObject.h"

//class k3dRenderQueue;

//! \class	k3dSceneManager
//! \brief	Manager for 3D scene.
class k3dSceneManager : public Singleton<k3dSceneManager>
{
public:

  /// \brief VMarkMap
  /// visible marks map
  typedef std::unordered_map<uint32, std::shared_ptr<std::vector<char> > > VMarkMap;

  k3dSceneManager();
  ~k3dSceneManager();

  void InitScene();
  void AddSceneObject(kSObject::SObjPtr objptr, const kMath::Vec3f &position = kMath::Vec3f(0.0f, 0.0f, 0.0f));
  void FrustumCull(k3dCamera *);
  void UpdateScene();

  bool LoadFromJSON(const char *scene_file);
  void SaveToJSON(const char* scene_file);

protected:

  k3dCamera*          m_CameraPtr;
  kSObject::SOVector  m_SceneVisibleObjs;
  kSObject::SOVector  m_SceneObjs;
  VMarkMap            m_VisibleMarkMap;

};

typedef std::shared_ptr<k3dSceneManager> SharedSceneManagerPtr;
