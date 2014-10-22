#pragma once
#ifndef __DCTranslator_h__
#define __DCTranslator_h__

#include "Maya.h"
#include <IIODevice.h>
#include <Core/k3dArchive.h>
#include <vector>
#include <json/json.h>

class k3dMesh;

class DCTranslator : public MPxFileTranslator
{
public:
  DCTranslator();
  virtual ~DCTranslator();

  static void*    creator();
  MStatus         reader (
      const MFileObject& file,
      const MString& optionsString,
      FileAccessMode mode);

  MStatus         writer (
      const MFileObject& file,
      const MString& optionsString,
      FileAccessMode mode );

  bool            haveReadMethod () const;
  bool            haveWriteMethod () const;
  MString         defaultExtension () const;

  MFileKind       identifyFile (
      const MFileObject& fileName,
      const char* buffer,
      short size) const;

private:
  MS            GetMeshFromNode(MDagPath &dagPath, MS &status, k3dMesh & mesh);
  MS            GetMaterialNode(MFnMesh & mesh, MS & status, Json::Value & matNode, MStringArray *uvSetNames = 0);
  MS            exportAll();
  MS            exportSelected();
  MStringArray*   objectNames;
  MStringArray    objectNodeNamesArray;

private:

  std::vector<k3dMesh*> m_Meshes;
  MString               m_MeshName;
  MString         m_ExportPath;
  k3dArchive*     m_MeshArch;
  IIODevice*      m_PhysxAssetFilePtr;
  IIODevice*      m_MeshFilePtr;
  IIODevice*      m_MaterialFilePtr;
  FILE*           m_LogFilePtr;
  Json::Value			m_MaterialRoot;
};

//////////////////////////////////////////////////////////////
#endif // K3DSTATICMESH_HPP
