#include "DCTranslator.h"
#include "Triangulation.h"

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnCamera.h>
#include <maya/MImage.h>
#include <maya/MItDag.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnSkinCluster.h>

#include <Core/MeshData.h>
#include <Core/CameraData.h>
#include <Core/LogUtil.h>

#include <tuple>
#include <iostream>
#include <fstream>

using namespace std;
using namespace k3d;

#define MeshWrite(ptr, size) m_MeshFilePtr->Write((ptr), (size))
#define ITER_I(LEN) for(int i=0; i<(LEN); i++)

static uint32 materialId = 0;

static std::string matAttributes[] = {
  "DiffuseColor", "SpecularColor", "AmbientColor", "EmissiveColor",
  "DiffuseMap", "SpecularMap", "AmbientMap", "NormalMap", "EmissiveMap", "TransparencyMap", "DisplaceMap",
  "Shininess", "Transparency"
};

DCTranslator::DCTranslator() {}

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
	m_ExportPath = file.rawPath();
#if K3DPLATFORM_OS_WIN
	m_Bundle = AssetBundle::CreateBundle(file.rawName().asWChar(), file.rawPath().asWChar());
#else
	m_Bundle = AssetBundle::CreateBundle(file.rawName().asChar(), file.rawPath().asChar());
#endif
	MGlobal::displayInfo(MString("Export Model Path: ") + m_ExportPath);
	m_Bundle->Prepare();
	if(m_Bundle)
	{
		if ((mode == MPxFileTranslator::kExportAccessMode) ||
			(mode == MPxFileTranslator::kSaveAccessMode))
		{
			exportAll();
		}
		else if (mode == MPxFileTranslator::kExportActiveAccessMode)
		{
			exportSelected();
		}
		m_Bundle->MergeAndBundle(true);
		delete m_Bundle;
		return MS::kSuccess;
	}
	else {
		return MStatus::kFailure;
	}
}

bool DCTranslator::InitExportFiles(MString const& fileBaseName)
{
	return false;
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

	if ((nameLength > 14) && !strcmp(name + nameLength - 7, MAYA_TRANSLATOR_EXT))
		return kCouldBeMyFileType;
	else
		return kNotMyFileType;
}

MS DCTranslator::exportAll()
{
	MStatus status = MS::kSuccess;
	MItDag dagIterator(MItDag::kBreadthFirst, MFn::kInvalid, &status);

	if (MS::kSuccess != status) 
	{
		fprintf(stderr, "Failure in DAG iterator setup.\n");
		return MS::kFailure;
	}
    
    MItDependencyNodes skinIter(MFn::kSkinClusterFilter);
    while (!skinIter.isDone())
    {
        status = MS::kSuccess;
        MObject object = skinIter.item();
        MFnSkinCluster skinCluster(object, &status);
        std::vector<MObject> objs;
        auto numMesh = skinCluster.numOutputConnections();
        for (auto i = 0; i < numMesh; ++ i)
        {
            auto index = skinCluster.indexForOutputConnection(i);
            objs.push_back(skinCluster.outputShapeAtIndex(index));
        }
        MDagPathArray influencePaths;
        MDagPath jointPath, rootPath;
        for (auto i = 0; i < skinCluster.influenceObjects(influencePaths, &status); ++ i)
        {
            jointPath = influencePaths[i];
            if (jointPath.hasFn(MFn::kJoint))
            {
                rootPath = jointPath;
                while (jointPath.length() > 0)
                {
                    jointPath.pop();
                    if (jointPath.hasFn(MFn::kJoint) && (jointPath.length() > 0))
                    {
                        rootPath = jointPath;
                    }
                }
                if (rootPath.hasFn(MFn::kJoint))
                {
                    MFnIkJoint fnJoint(rootPath);
                }
            }
        }
        skinIter.next();
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
			auto pMesh = std::make_unique<MeshData>();
			status = GetMeshFromNode(dagPath, status, *pMesh);
			//WriteTheMesh Here
			if (status == MS::kSuccess)
			{
				m_Bundle->Serialize(pMesh.get());
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
				MFnCamera camera(dagPath, &status);
				MGlobal::displayInfo(MString("exporting camera \"") + camera.name() + "\"");
				CameraData cameraData;
				cameraData.SetName(camera.name().asChar());
				cameraData.SetFOV(camera.verticalFieldOfView());
				cameraData.SetFocalLength(camera.focalLength());
				cameraData.SetNearPlane(camera.nearClippingPlane());
				cameraData.SetFarPlane(camera.farClippingPlane());
				auto v = camera.upDirection(MSpace::kWorld);
				cameraData.SetUpVector({(float)v.x,(float)v.y,(float)v.z});
				v = camera.rightDirection(MSpace::kWorld);
				cameraData.SetRightVector({ (float)v.x,(float)v.y,(float)v.z });
				v = camera.viewDirection(MSpace::kWorld);
				cameraData.SetViewVector({ (float)v.x,(float)v.y,(float)v.z });
				v = camera.eyePoint(MSpace::kWorld);
				cameraData.SetPosition({ (float)v.x,(float)v.y,(float)v.z });
				m_Bundle->Serialize(&cameraData);
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
	if (selIt.isDone())
    {
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
					auto pMesh = std::make_unique<MeshData>();
					status = GetMeshFromNode(dagPath, status, *pMesh);
					//WriteTheMesh Here
					if (status == MS::kSuccess) 
					{
						m_Bundle->Serialize(pMesh.get());
						MGlobal::displayInfo("Write Mesh finished.");
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

MS DCTranslator::GetMeshFromNode(MDagPath &dagPath, MS &status, MeshData &mesh) {

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

		///////// Triangulation ///////////////
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

		mesh.SetPrimType(PrimType::TRIANGLES);

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
