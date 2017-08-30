#pragma once
#ifndef __DCTranslator_h__
#define __DCTranslator_h__

#include "MayaCommon.h"

#include <Core/Interface/IIODevice.h>
#include <Core/KTL/Archive.hpp>
#include <Engine/Assets/MeshData.h>
#include <Engine/Assets/CameraData.h>
#include <Engine/Assets/Bundle.h>
#include <vector>

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
	bool			InitExportFiles(MString const& fileBaseName);
	MS				GetMeshFromNode(MDagPath &dagPath, MS &status, k3d::MeshData & mesh);
	//MS            GetMaterialNode(MFnMesh & mesh, MS & status, Json::Value & matNode, MStringArray *uvSetNames = 0);
	MS				exportAll();
	MS				exportSelected();
	MStringArray*   objectNames;
	MStringArray    objectNodeNamesArray;

private:
	using VecSpCamera	= std::vector<k3d::SpCameraData>;
	VecSpCamera			m_Cameras;

	k3d::AssetBundle*	m_Bundle;

	MString             m_MeshName;
	MString				m_ExportPath;
};

//////////////////////////////////////////////////////////////
#endif // K3DSTATICMESH_HPP
