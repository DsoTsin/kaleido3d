#include "k3dMaterialManager.h"


k3dMaterialManager::k3dMaterialManager()
{
}

k3dMaterialManager::~k3dMaterialManager()
{
}

void k3dMaterialManager::LoadMaterial(const char *mat_file_name)
{

}

std::shared_ptr<k3dMaterial> k3dMaterialManager::FindMaterialByName(const char *name)
{
  k3dString matName(name);
  return FindMaterialByName(matName);
}

std::shared_ptr<k3dMaterial> k3dMaterialManager::FindMaterialByName(const k3dString &name)
{
  MaterialMap::const_iterator iter = m_Materials.find(name);
  if(iter != m_Materials.end()) {
    return m_Materials[name];
  }
  else
    return nullptr;
}
