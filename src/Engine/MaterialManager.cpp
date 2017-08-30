#include "Kaleido3D.h"
#include "MaterialManager.h"
#include "Material.h"

namespace k3d
{

	MaterialManager::MaterialManager()
	{
	}

	MaterialManager::~MaterialManager()
	{
	}

	void MaterialManager::LoadMaterial(const char *mat_file_name)
	{

	}

	SharedPtr<Material> MaterialManager::FindMaterialByName(const char *name)
	{
		std::string matName(name);
		return FindMaterialByName(matName);
	}

	SharedPtr<Material> MaterialManager::FindMaterialByName(const std::string & name)
	{
		MaterialMap::const_iterator iter = m_Materials.find(name);
		if (iter != m_Materials.end()) {
			return m_Materials[name];
		}
		else
			return nullptr;
	}
}
