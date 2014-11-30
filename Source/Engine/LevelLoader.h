#pragma once
#include <Math/kMath.hpp>

class k3dSceneManager;

class k3dLevelLoader {
public:
  k3dLevelLoader( std::shared_ptr<k3dSceneManager> scene );
  ~k3dLevelLoader();

  bool LoadLevel(const char* levelFile);

private:

  std::shared_ptr<k3dSceneManager> m_Scene;
};
