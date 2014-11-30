#pragma once
#ifndef __DCTranslator_h__
#define __DCTranslator_h__

#include "Maya.h"
#include <Interface/IIODevice.h>
#include <Core/Archive.h>
#include <Core/Mesh.h>
#include <vector>

namespace k3d {
	class Mesh;
}

class DCTranslator : public MPxFileTranslator
{
public:
	DCTranslator();
	virtual ~DCTranslator();

	static void*    creator();
	MStatus         reader(const MFileObject& file, const MString& optionsString, FileAccessMode mode) override;
	MStatus         writer(const MFileObject& file, const MString& optionsString, FileAccessMode mode) override;
	bool            haveReadMethod() const override;
	bool            haveWriteMethod() const override;
	MString         defaultExtension() const override;
	MFileKind       identifyFile(const MFileObject& fileName, const char* buffer, short size) const override;

private:
	MS            GetMeshFromNode(MDagPath &dagPath, MS &status, k3d::Mesh & mesh);
	//MS            GetMaterialNode(MFnMesh & mesh, MS & status, Json::Value & matNode, MStringArray *uvSetNames = 0);
	MS            exportAll();
	MS            exportSelected();
	MStringArray*   objectNames;
	MStringArray    objectNodeNamesArray;

private:
	typedef std::vector<k3d::SpMesh> VecSpMesh;
	VecSpMesh			m_Meshes;
	MString             m_MeshName;
	MString				m_ExportPath;
	k3d::Archive*		m_MeshArch;
	//IIODevice*      m_PhysxAssetFilePtr;
	IIODevice*			m_MeshFilePtr;
	IIODevice*			m_MaterialFilePtr;
	//Json::Value			m_MaterialRoot;
};

//////////////////////////////////////////////////////////////
#endif // K3DSTATICMESH_HPP
