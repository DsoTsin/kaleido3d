#pragma once

#include <Core/Kaleido3D.h>
#include <Core/Os.h>

#include <maya/MMatrix.h>
#include <maya/MBoundingBox.h>
#include <maya/MPxFileTranslator.h>

#include <maya/MPxCommand.h>
#include <maya/MStatus.h>

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#include <maya/MFloatPointArray.h>

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include <maya/MPoint.h>
#include <maya/MPointArray.h>

#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>

#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MObjectArray.h>
#include <maya/MObject.h>

#include <maya/MPxNode.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericData.h>

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>

#include <maya/MDistance.h>
#include <maya/MIntArray.h>
#include <maya/MIOStream.h>

#define MAYA_PLUGIN_COMPANY			"Tsin Studio"
#define MAYA_PLUGIN_VERSION			"1.1.1"
#define MAYA_PLUGIN_REQUIREDAPI		"Any"

#define MAYA_TRANSLATOR				"kaleido3d digital content package"
#define MAYA_TRANSLATOR_EXT			"bundle"

#define MS_CHECK(status, stat) \
if (status != MStatus::kSuccess) \
{\
  status.perror(#stat); \
}