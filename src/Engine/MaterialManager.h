#pragma once
#include "Material.h"
#include <KTL/Singleton.hpp>
#include <memory>
#include <map>

namespace k3d
{
	class Material;

	///
	/// \brief The k3dMaterialManager class manages material loading, finding
	///
	class MaterialManager : public Singleton<MaterialManager> {
		typedef std::map<std::string, SharedPtr<Material> >  MaterialMap;
	public:
		MaterialManager();
		~MaterialManager();

		///
		/// \brief LoadMaterial
		/// \param mat_file_name
		///
		void LoadMaterial(const char * mat_file_name);

		///
		/// \brief FindMaterialByName
		/// \param name
		/// \return
		///
		SharedPtr<Material>
			FindMaterialByName(const char * name);

		///
		/// \brief FindMaterialByName
		/// \param name
		/// \return
		///
		SharedPtr<Material>
			FindMaterialByName(const std::string & name);

	private:
		MaterialMap     m_Materials;
	};
}
