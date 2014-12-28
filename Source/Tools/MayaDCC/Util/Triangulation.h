#pragma once
#include "Maya.h"
#include <Core/Mesh.h>
#include <vector>

int Triangulation(
	MFnMesh &mfnMesh,
	MIntArray &triangleCount, MIntArray &triangleList, MIntArray &triangleNList, MIntArray &triangleUVList,
	MIntArray &vertexCount, MIntArray &vertexList, MIntArray &normalList, MIntArray &uvIds);


void RemapToOGLFmt(
	MFnMesh &mfnMesh,
	k3d::Mesh & outputMesh,
	MIntArray & triangleList, 
	MIntArray & triangleNList, 
	MIntArray & triangleUVList,
	MString & uvSetName);