#pragma once
#include <Core/k3dMaterial.h>
#include <KTL/Singleton.h>

///
/// \brief The k3dMaterialManager class manages material loading, finding
///
class k3dMaterialManager : public Singleton<k3dMaterialManager> {
  typedef std::map<k3dString, std::shared_ptr<k3dMaterial> >  MaterialMap;
public:
  k3dMaterialManager();
  ~k3dMaterialManager();

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
  std::shared_ptr<k3dMaterial>
  FindMaterialByName(const char * name);

  ///
  /// \brief FindMaterialByName
  /// \param name
  /// \return
  ///
  std::shared_ptr<k3dMaterial>
  FindMaterialByName(const k3dString & name);

private:
  MaterialMap     m_Materials;
}
