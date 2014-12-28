#include "DCTranslator.h"
#include "Util/Triangulation.h"

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnCamera.h>
#include <maya/MImage.h>

#include <Core/File.h>
#include <Core/Mesh.h>
#include <Core/LogUtil.h>

#include <tuple>
#include <iostream>
#include <fstream>

using namespace std;
using namespace k3d;

#define MeshWrite(ptr, size) m_MeshFilePtr->Write((ptr), (size))
#define ITER_I(LEN) for(int i=0; i<(LEN); i++)

static uint32 materialId = 0;

static k3dString matAttributes[] = {
  "DiffuseColor", "SpecularColor", "AmbientColor", "EmissiveColor",
  "DiffuseMap", "SpecularMap", "AmbientMap", "NormalMap", "EmissiveMap", "TransparencyMap", "DisplaceMap",
  "Shininess", "Transparency"
};

DCTranslator::DCTranslator() : m_MeshFilePtr(0) {}

DCTranslator::~DCTranslator() { }

void *DCTranslator::creator() { return new DCTranslator; }

MStatus DCTranslator::reader(
	const MFileObject &,
	const MString &,
	MPxFileTranslator::FileAccessMode)
{
	fprintf(stderr, "K3DStaticMesh::reader function not Implemented!\n");
	return MS::kFailure;
}

MStatus DCTranslator::writer(
	const MFileObject &file,
	const MString &,
	MPxFileTranslator::FileAccessMode mode)
{
	MString fileName = file.fullName();
	m_ExportPath = file.path();
	MString pathInfo = "Export Model Path: ";
	MGlobal::displayInfo(pathInfo + m_ExportPath);
	MGlobal::displayInfo("Exporting Mesh...\n");

	m_MeshFilePtr = GetIODevice<File>();
	//m_PhysxAssetFilePtr = GetIODevice<File>();
	if (m_MeshFilePtr->Open(fileName.asChar(), IOWrite)) {
		//m_PhysxAssetFilePtr->Open((fileName+".pxasset").asChar(), IOWrite);
		m_MeshArch = new Archive;
		m_MeshArch->SetIODevice(m_MeshFilePtr);
		//////         write the header            ///////
		MeshHeader header;
		header.Version = VERSION_1_1;
		//////
		(*m_MeshArch) << header;

		if ((mode == MPxFileTranslator::kExportAccessMode) ||
			(mode == MPxFileTranslator::kSaveAccessMode))
		{
			exportAll();
		}
		else if (mode == MPxFileTranslator::kExportActiveAccessMode)
		{
			exportSelected();
		}

		char EndCode[64] = "End";
		m_MeshFilePtr->Write(EndCode, 64);
		m_MeshFilePtr->Close();
		delete m_MeshFilePtr;
		m_MeshFilePtr = 0;

		return MS::kSuccess;
	}
	else {
		return MStatus::kFailure;
	}
}

bool DCTranslator::haveReadMethod() const
{
	return false;
}

bool DCTranslator::haveWriteMethod() const
{
	return true;
}

MString DCTranslator::defaultExtension() const
{
	return MAYA_TRANSLATOR_EXT;
}

MPxFileTranslator::MFileKind DCTranslator::identifyFile(
	const MFileObject &fileName, const char *, short) const
{
	const char * name = fileName.name().asChar();
	size_t   nameLength = strlen(name);

	if ((nameLength > 14) && !_stricmp(name + nameLength - 7, ".kscene"))
		return kCouldBeMyFileType;
	else
		return kNotMyFileType;
}

MS DCTranslator::exportAll()
{
	MStatus status = MS::kSuccess;
	MItDag dagIterator(MItDag::kBreadthFirst, MFn::kInvalid, &status);

	if (MS::kSuccess != status) {
		fprintf(stderr, "Failure in DAG iterator setup.\n");
		return MS::kFailure;
	}

	for (; !dagIterator.isDone(); dagIterator.next())
	{
		MDagPath dagPath;
		status = dagIterator.getPath(dagPath);

		if (!status) {
			fprintf(stderr, "Failure getting DAG path.\n");
			return MS::kFailure;
		}

		MFnDagNode dagNode(dagPath, &status);
		if (dagNode.isIntermediateObject())
		{
			continue;
		}

		if ((dagPath.hasFn(MFn::kNurbsSurface)) &&
			(dagPath.hasFn(MFn::kTransform)))
		{
			status = MS::kSuccess;
			fprintf(stderr, "Warning: skipping Nurbs Surface.\n");
		}
		else if ((dagPath.hasFn(MFn::kMesh)) &&
			(dagPath.hasFn(MFn::kTransform)))
		{
			// We want only the shape,
			// not the transform-extended-to-shape.
			continue;
		}
		else if (dagPath.hasFn(MFn::kMesh))
		{
			SpMesh curMesh{ new Mesh };
			status = GetMeshFromNode(dagPath, status, *curMesh);
			m_Meshes.push_back(curMesh);
			//WriteTheMesh Here
			if (status == MS::kSuccess) {
				(*m_MeshArch) << (*curMesh);
				MGlobal::displayInfo("Write Mesh finished...\n");
			}
			else {
				MGlobal::displayError("Error GetMesh From Node...\n");
			}
		}
		else if (dagPath.hasFn(MFn::kCamera))
		{
			if (!dagPath.hasFn(MFn::kTransform))
			{
				MGlobal::displayInfo("Find Camera Info.");
				MFnCamera camera(dagPath, &status);
				MPoint point = camera.eyePoint(MSpace::kWorld, &status);
				if (status == MS::kSuccess) {
					MGlobal::displayInfo(MString("camera x=")+point.x);
				}
			}
		}
	}
	return status;
}

MS DCTranslator::exportSelected()
{
	MS status;
	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);
	MItSelectionList selIt(selection);
	if (selIt.isDone()) {
		status.perror("MayaToolKit: Nothing Selected!");
		MGlobal::displayWarning("kaleido maya toolkit: Nothing selected!");
		return MS::kFailure;
	}

	MDagPathArray pathArray;
	for (; !selIt.isDone(); selIt.next()) {
		MItDag dagIt(MItDag::kDepthFirst, MFn::kInvalid, &status);
		MDagPath objPath;
		status = selIt.getDagPath(objPath);
		status = dagIt.reset(objPath.node(), MItDag::kDepthFirst, MFn::kInvalid);
		do {
			MDagPath dagPath;
			MObject component = MObject::kNullObj;
			status = dagIt.getPath(dagPath);
			MFnDagNode dagNode(dagPath, &status);
			if (dagNode.isIntermediateObject())
			{

			}
			else if (dagPath.hasFn(MFn::kMesh))
			{
				if (!dagPath.hasFn(MFn::kTransform))
				{
					SpMesh curMesh{ new Mesh };
					status = GetMeshFromNode(dagPath, status, *curMesh);
					//WriteTheMesh Here
					if (status == MS::kSuccess) {
						(*m_MeshArch) << curMesh;
						MGlobal::displayInfo("Write Mesh finished.");
						m_Meshes.push_back(curMesh);
					}
					else {
						MGlobal::displayError("Error GetMesh From Node.");
					}
				}
			}
			else if (dagPath.hasFn(MFn::kCamera))
			{
				if (!dagPath.hasFn(MFn::kTransform))
				{
					pathArray.append(dagPath);
				}
			}
			else if (dagPath.hasFn(MFn::kSpotLight))
			{
				if (!dagPath.hasFn(MFn::kTransform))
				{
					pathArray.append(dagPath);
				}
			}
			dagIt.next();
		} while (!dagIt.isDone());
	}

	return status;
}


MS DCTranslator::GetMeshFromNode(MDagPath &dagPath, MS &status, Mesh &mesh) {

	MFnMesh mfnMesh(dagPath, &status);
	m_MeshName = mfnMesh.name(&status);
	MGlobal::displayInfo(MString("exporting mesh \"") + m_MeshName + "\"");
	mesh.SetMeshName(m_MeshName.asChar());
	if (MS::kSuccess == status) {
		MIntArray vertexCount, vertexList, normalCount, normalList, uvCounts, uvIds;
		mfnMesh.getAssignedUVs(uvCounts, uvIds);
		MIntArray triangleCount, triangleList, triangleNList, triangleUVList;
		mfnMesh.getVertices(vertexCount, vertexList);
		mfnMesh.getNormalIds(normalCount, normalList);
		mfnMesh.getTriangles(triangleCount, triangleList);
		triangleNList.setLength(triangleList.length());
		triangleUVList.setLength(triangleList.length());

		///////// Triangulation ////////////////
		Triangulation(
			mfnMesh,
			triangleCount, triangleList, triangleNList, triangleUVList,
			vertexCount, vertexList, normalList, uvIds);
		/////////End of Triangulation ////////////////////
		
		/*
		MFnDagNode dagNode(dagPath);
		MBoundingBox bbox = dagNode.boundingBox();
		float maxCorner[3]; float minCorner[3];
		bbox.max().get(maxCorner);
		bbox.min().get(minCorner);
		mesh.SetBBox(maxCorner, minCorner);

		//info.NumTriangles = tri_idx_offset / 3;
		//Exports Material
		Json::Value matNode;
			matNode["MatId"] = materialId;
			Json::Value matSubLayer;*/
			/*GetMaterialNode(mfnMesh, mats, matSubLayer, &uvSets);
			matNode["Layer"] = matSubLayer;
		m_MaterialRoot.append(matNode);
		*/

		MStringArray uvSets;
		RemapToOGLFmt(mfnMesh, mesh,
			triangleList, triangleNList, triangleUVList, uvSets[0]);

		mesh.SetPrimType(TRIANGLES);

	}

	if (status != MS::kSuccess) {
		MGlobal::displayError("GetMeshFormNode Error: exporting geom failed.\n");
		return MS::kFailure;
	}

	return status;
}
//
//MS DCTranslator::GetMaterialNode(MFnMesh &mesh, MS &status, Json::Value &matNode, MStringArray *_uvSetNames)
//{
//  int shaderNum = 0;
//  MObjectArray shaders;
//  MIntArray shaderIndices;
//  mesh.getConnectedShaders(shaderNum, shaders, shaderIndices);
//  //Get UVSet and associated textures
//  MObjectArray textures;
//  MStringArray uvSetNames;
//  mesh.getUVSetNames(uvSetNames);
//  MString *uvName;
//  if (uvSetNames.length() > 0) {
//    uvName = &uvSetNames[0];
//    mesh.getAssociatedUVSetTextures(*uvName, textures);
//  }
//
//  if (_uvSetNames) *_uvSetNames = uvSetNames;
//
//  fprintf(m_LogFilePtr, "Material[%d] UvSets{%s", materialId, uvName->asChar());
//  for (int i = 1; i<uvSetNames.length(); i++)
//    fprintf(m_LogFilePtr, ",%s", uvSetNames[i].asChar());
//  fprintf(m_LogFilePtr, "}\n");
//
//  //Export Material
//  if (shaders.length() > 0) {
//    kDebug("Shader Length: %d\n", shaders.length() );
//    for(int si=0; si<shaders.length(); si++) {
//      MFnDependencyNode shaderNode(shaders[si]);
//      MPlugArray connections;
//      MObject surfShader;
//      bool hasSurfShader = false;
//
//      Json::Value subMatLayer;
//      //! Default Set To Opaque Material
//      subMatLayer["MaterialType"] = k3dString("Opaque");
//
//      if (shaders[si].hasFn(MFn::kShadingEngine)) {
//        MPlug plug_surf_shader = shaderNode.findPlug("surfaceShader", true);
//        plug_surf_shader.connectedTo(connections, true, false);
//        kDebug("Connection Length: %d\n", connections.length() );
//        if (connections.length() > 0) {
//          surfShader = connections[0].node();
//          hasSurfShader = true;
//          if (surfShader.hasFn(MFn::kLambert))
//          {
//            MFnLambertShader lambert(surfShader);
//            MColor ac = lambert.ambientColor();
//            MColor dc = lambert.color();
//            MColor ec = lambert.incandescence();
//            MColor tr = lambert.transparency();
//            float dcoeff = lambert.diffuseCoeff();
//
//            MColor spec(1.0f, 1.0f, 1.0f, 1.0f);
//            float shininess = 32.0f;
//            if (surfShader.hasFn(MFn::kPhong))
//            {
//              MFnPhongShader phong(surfShader);
//              spec = phong.specularColor();
//              shininess = phong.cosPower();
//            }
//            else if (surfShader.hasFn(MFn::kBlinn))
//            {
//              MFnBlinnShader blinn(surfShader);
//              spec = blinn.specularColor();
//              shininess = blinn.specularRollOff();
//            }
//
//            // base material param should contain ADSE
//            subMatLayer["AmbientColor"] = Vec3fToJV(kMath::Vec3f(ac.r, ac.g, ac.b));
//            subMatLayer["DiffuseColor"] = Vec3fToJV(kMath::Vec3f(dcoeff*dc.r, dcoeff*dc.g, dcoeff*dc.b));
//            subMatLayer["SpecularColor"] = Vec3fToJV(kMath::Vec3f(spec.r, spec.g, spec.b));
//            subMatLayer["EmissiveColor"] = Vec3fToJV(kMath::Vec3f(ec.r, ec.g, ec.b));
//            subMatLayer["Shininess"]    = shininess;
//
//            float transparency = 1.0f - (tr.r + tr.g + tr.b) / 3.0f;
//            if(transparency-0.999f<=0.00001f) {
//              subMatLayer["MaterialType"] = k3dString("Translucent");
//              subMatLayer["Transparency"] = transparency;
//            }
//
//          }
//          else
//          {
//            MGlobal::displayError("Unknown material type.");
//          }
//        }
//      }
//      std::map<std::string, std::string> texture_type_map = {
//        { "DiffuseMap", "color" },
//        { "AmbientMap", "ambientColor" },
//        { "SpecularMap", "specularColor" },
//        { "EmitMap", "incandescene" },
//        { "TransparencyMap", "transparency" },
//        { "NormalMap", "bumpValue" }
//      };
//      if (hasSurfShader)
//      {
//        MFnDependencyNode surface_dn(surfShader);
//        for (std::map<std::string, std::string>::const_iterator iter = texture_type_map.begin();
//             iter != texture_type_map.end(); ++iter)
//        {
//          MPlug plug_specified = surface_dn.findPlug(iter->second.c_str(), true);
//          if (!plug_specified.isConnected())
//          {
//            continue;
//          }
//
//          connections.clear();
//          plug_specified.connectedTo(connections, true, false);
//          if (connections.length() > 0)
//          {
//            MObject tex_obj = connections[0].node();
//            if (tex_obj.hasFn(MFn::kFileTexture))
//            {
//              MFnDependencyNode tex_dn(tex_obj);
//              MPlug plug = tex_dn.findPlug("fileTextureName");
//              MString texture_name;
//              plug.getValue(texture_name);
//
//              if(iter->first=="TransparencyMap") {
//                subMatLayer["MaterialType"] = k3dString("Translucent");
//              }
//              else if( iter->first=="DiffuseMap" || iter->first=="NormalMap" ) {
//                MImage tex; tex.readFromFile(texture_name);
//                if(tex.isRGBA()) {
//                  subMatLayer["MaterialType"] = k3dString("Mask");
//                }
//              }
//
//              subMatLayer[iter->first] = string(texture_name.asChar());
//            }
//            else
//            {
//              MGlobal::displayError("Unknown texture data type, not a valid file texture.");
//            }
//          }
//        }
//      }
//      matNode.append(subMatLayer);
//    } // end for for
//  }// end for if
//  materialId++;
//  return status;
//}
