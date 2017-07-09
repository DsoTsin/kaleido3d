#include "Triangulation.h"

#pragma warning(disable:4267) // size_t->int

using namespace k3d;

int Triangulation(
	MFnMesh &mfnMesh,
	MIntArray &triangleCount, MIntArray &triangleList, MIntArray &triangleNList, MIntArray &triangleUVList,
	MIntArray &vertexCount, MIntArray &vertexList, MIntArray &normalList, MIntArray &uvIds)
{
	int poly_idx_offset = 0;
	int tri_idx_offset = 0;
	for (int i = 0; i < mfnMesh.numPolygons(); ++i)
	{
		for (int j = 0; j < triangleCount[i]; ++j)
		{
			for (unsigned int k = 0; k < 3; ++k)
			{
				int v_idx = triangleList[tri_idx_offset + j * 3 + k];
				int match = -1;
				int l = 0;
				while (match < 0 && l < vertexCount[i])
				{
					if (vertexList[poly_idx_offset + l] == v_idx)
						match = l;
					++l;
				}
				triangleNList[tri_idx_offset + j * 3 + k] = normalList[poly_idx_offset + match];
				int id = 0;
				if (uvIds.length() != 0)
					mfnMesh.getPolygonUVid(i, match, id);
				triangleUVList[tri_idx_offset + j * 3 + k] = id;
			}
		}
		poly_idx_offset += vertexCount[i];
		tri_idx_offset += 3 * triangleCount[i];
	}
	return tri_idx_offset;
}

void RemapToOGLFmt(
	MFnMesh &mfnMesh,
	k3d::MeshData & outputMesh,
	MIntArray & triangleList, 
	MIntArray & triangleNList, 
	MIntArray & triangleUVList,
	MString & uvSetName) {
	assert(triangleList.length() == triangleNList.length());
	assert(triangleNList.length() == triangleUVList.length());

	typedef std::tuple<int, int, int> P_N_T;
	typedef std::map<P_N_T, int> PNTMap;
	typedef PNTMap::const_iterator PNTIter;

	PNTMap  h;
	MFloatPointArray vertex; mfnMesh.getPoints(vertex);
	MFloatVectorArray normal; mfnMesh.getNormals(normal);
	MFloatArray uArray, vArray; mfnMesh.getUVs(uArray, vArray, &uvSetName);

	bool hasUV = (uArray.length() > 0);
	bool hasNormal = (normal.length() > 0);
	
	VtxFormat defaultVtxFmt = VtxFormat::POS3_F32;
	if (hasNormal && hasUV) {
		defaultVtxFmt = VtxFormat::POS3_F32_NOR3_F32_UV2_F32;
	}
	else if (hasNormal && !hasUV) {
		defaultVtxFmt = VtxFormat::POS3_F32_NOR3_F32;
	}
	else if (hasUV && !hasNormal) {
		defaultVtxFmt = VtxFormat::POS3_F32_UV2_F32;
	}
	outputMesh.SetVertexFormat(defaultVtxFmt);

	std::vector<uint32> indexBuffer;

	std::vector<k3d::Vertex3F3F2F> vertex332List;
	std::vector<k3d::Vertex3F3F>   vertex33List;
	std::vector<k3d::Vertex3F>	   vertex3List;

	for (size_t i = 0, idx = 0; i < triangleList.length(); i++)
	{
		P_N_T p(triangleList[i], triangleNList[i], triangleUVList[i]);
		PNTIter match = h.find(p);
		// found
		if (match != h.end())
		{
			indexBuffer.push_back(match->second);
		}
		else //Not Found
		{
			h[p] = idx;
			indexBuffer.push_back(idx++);
			int vid = std::get<0>(p); 
			int nid = std::get<1>(p); 
			int tid = std::get<2>(p);
			if (defaultVtxFmt == VtxFormat::POS3_F32_NOR3_F32_UV2_F32) {
				vertex332List.push_back({
					vertex[vid].x, vertex[vid].y, vertex[vid].z,
					normal[nid].x, normal[nid].y, normal[nid].z,
					uArray[tid], vArray[tid]
				});
			}
			else if (defaultVtxFmt == VtxFormat::POS3_F32_NOR3_F32) {
				vertex33List.push_back({
					vertex[vid].x, vertex[vid].y, vertex[vid].z,
					normal[nid].x, normal[nid].y, normal[nid].z
				});
			}
			else if (defaultVtxFmt == VtxFormat::POS3_F32) {
				vertex3List.push_back({
					vertex[vid].x, vertex[vid].y, vertex[vid].z,
				});
			}
		}
	}

	outputMesh.SetIndexBuffer(indexBuffer);
	if (defaultVtxFmt == VtxFormat::POS3_F32_NOR3_F32_UV2_F32) {
		outputMesh.SetVertexNum(vertex332List.size());
		outputMesh.SetVertexBuffer(&vertex332List[0]);
	}
	else if (defaultVtxFmt == VtxFormat::POS3_F32_NOR3_F32) {
		outputMesh.SetVertexNum(vertex33List.size());
		outputMesh.SetVertexBuffer(&vertex33List[0]);
	}
	else if (defaultVtxFmt == VtxFormat::POS3_F32) {
		outputMesh.SetVertexNum(vertex3List.size());
		outputMesh.SetVertexBuffer(&vertex3List[0]);
	}

}