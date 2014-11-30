#include "Mesh.h"
#include <assert.h>
#include <Core/Archive.h>

namespace k3d {

#define SAFERELEASEARRAY(x) \
  if(x) {\
  delete [] x;\
  x=nullptr;\
  }

	Mesh::Mesh()
	{
		m_IsLoaded = false;
		m_NumIndices = 0;
		m_NumVertices = 0;
		m_NumNormals = 0;
		m_NumTangents = 0;
		m_NumUVs = 0;

		m_IndexData = nullptr;
		m_VertexData = nullptr;
		m_NormalData = nullptr;
		m_TangentsData = nullptr;
		m_UVData = nullptr;
		memset(m_MeshName, 0, 96);
	}

	Mesh::~Mesh()
	{
		Release();
	}

	void Mesh::Release()
	{
		SAFERELEASEARRAY(m_IndexData);
		SAFERELEASEARRAY(m_VertexData);
		SAFERELEASEARRAY(m_NormalData);
		SAFERELEASEARRAY(m_TangentsData);
		SAFERELEASEARRAY(m_UVData);
		m_IsLoaded = false;
		m_NumIndices = 0;
		m_NumVertices = 0;
		m_NumNormals = 0;
		m_NumTangents = 0;
		m_NumUVs = 0;
	}

	void Mesh::SetMeshName(const char *meshName)
	{
		assert(meshName && "MeshName cannot be nullptr");
		const char * ptr = meshName;
		int pos = 0;
		while (*ptr != 0 && pos < 96) {
			m_MeshName[pos] = *ptr;
			ptr++; pos++;
		}
	}

	void Mesh::SetBBox(float maxCorner[], float minCorner[])
	{
		m_MaxCorner.init(maxCorner);
		m_MinCorner.init(minCorner);
	}

	void Mesh::SetIndexBuffer(std::vector<int> &indexBuffer)
	{
		m_NumIndices = (uint32)indexBuffer.size();
		if (m_NumIndices != 0) {
			m_IndexData = new uint32[m_NumIndices];
			memcpy(m_IndexData, (uint32*)&std::move(indexBuffer)[0], m_NumIndices*sizeof(uint32));
			//    memc
		}
	}

	void Mesh::SetVertexBuffer(std::vector<kMath::Vec3f> &vertexBuffer)
	{
		m_NumVertices = (uint32)vertexBuffer.size();
		if (m_NumVertices != 0) {
			m_VertexData = new kMath::Vec3f[m_NumVertices];
			memcpy(m_VertexData, (kMath::Vec3f*)&std::move(vertexBuffer)[0], m_NumVertices*sizeof(kMath::Vec3f));
		}
	}

	void Mesh::SetNormalBuffer(std::vector<kMath::Vec3f> &normalBuffer)
	{
		m_NumNormals = (uint32)normalBuffer.size();
		if (m_NumNormals != 0) {
			m_NormalData = new kMath::Vec3f[m_NumIndices];
			memcpy(m_NormalData, (kMath::Vec3f*)&std::move(normalBuffer)[0], m_NumNormals*sizeof(kMath::Vec3f));
		}
	}

	void Mesh::SetUVBuffer(std::vector<kMath::Vec2f> & uvBuffer)
	{
		m_NumUVs = (uint32)uvBuffer.size();
		if (m_NumNormals != 0) {
			m_UVData = new kMath::Vec2f[m_NumUVs];
			memcpy(m_UVData, (kMath::Vec2f*)&std::move(uvBuffer)[0], m_NumUVs*sizeof(kMath::Vec2f));
		}
	}

	void Mesh::SetMaterialID(uint32 matID)
	{
		m_MaterialID = matID;
	}

	bool Mesh::IsLoaded() const
	{
		return m_IsLoaded;
	}

	bool Mesh::HasNormalData() const
	{
		return (m_NumNormals != 0);
	}

	bool Mesh::HasTangentData() const
	{
		return (m_NumTangents != 0);
	}

	kMath::AABB Mesh::GetBoundingBox() const
	{
		return kMath::AABB(m_MaxCorner, m_MinCorner);
	}

	int Mesh::GetVertexNum() const
	{
		return m_NumVertices;
	}

	int Mesh::GetIndexNum() const
	{
		return m_NumIndices;
	}

	int Mesh::GetTrianglesNum() const
	{
		return m_NumIndices / 3;
	}

	uint32 Mesh::GetMaterialID() const
	{
		return m_MaterialID;
	}

	float * Mesh::GetTangentBuffer() const
	{
		assert(m_TangentsData && "m_TangentData == nullptr !");
		return (float*)&m_TangentsData[0];
	}

	float * Mesh::GetTexCoordBuffer() const
	{
		assert(m_UVData && "m_UVData == nullptr !");
		return (float*)&m_UVData[0];
	}

	float * Mesh::GetNormalBuffer() const
	{
		assert(m_NormalData && "m_NormalData == nullptr !");
		return (float*)&m_NormalData[0];
	}

	float *Mesh::GetVertexBuffer() const
	{
		assert(m_VertexData && "m_VertexData == nullptr!!");
		return (float*)&m_VertexData[0];
	}

	uint32 *Mesh::GetIndexBuffer() const
	{
		assert(m_IndexData && "m_IndexData == nullptr!!");
		return m_IndexData;
	}

	const char * Mesh::MeshName() const
	{
		return m_MeshName;
	}

	Archive & operator >>(Archive &arch, Mesh &mesh)
	{
		//  arch.ArrayIn(Mesh::ClassName(), 64);
		arch.ArrayOut(mesh.m_MeshName, 96);
		arch >> mesh.m_NumIndices;
		arch >> mesh.m_NumVertices;
		arch >> mesh.m_NumNormals;
		arch >> mesh.m_NumTangents;
		arch >> mesh.m_NumUVs;
		arch >> mesh.m_MaterialID;
		arch >> mesh.m_MaxCorner;
		arch >> mesh.m_MinCorner;

		if (mesh.m_NumIndices != 0) {
			mesh.m_IndexData = new uint32[mesh.m_NumIndices];
			arch.ArrayOut(mesh.m_IndexData, mesh.m_NumIndices);
		}

		if (mesh.m_NumVertices != 0) {
			mesh.m_VertexData = new kMath::Vec3f[mesh.m_NumVertices];
			arch.ArrayOut(mesh.m_VertexData, mesh.m_NumVertices);
		}

		if (mesh.m_NumNormals != 0) {
			mesh.m_NormalData = new kMath::Vec3f[mesh.m_NumNormals];
			arch.ArrayOut(mesh.m_NormalData, mesh.m_NumNormals);
		}

		if (mesh.m_NumTangents != 0) {
			mesh.m_TangentsData = new kMath::Vec3f[mesh.m_NumTangents];
			arch.ArrayOut(mesh.m_TangentsData, mesh.m_NumTangents);
		}

		if (mesh.m_NumUVs != 0) {
			mesh.m_UVData = new kMath::Vec2f[mesh.m_NumUVs];
			arch.ArrayOut(mesh.m_UVData, mesh.m_NumUVs);
		}

		return arch;
	}

	Archive & operator <<(Archive &arch, const Mesh &mesh)
	{
		arch.ArrayIn(Mesh::ClassName(), 64);
		arch.ArrayIn(mesh.m_MeshName, 96);
		arch << mesh.m_NumIndices;
		arch << mesh.m_NumVertices;
		arch << mesh.m_NumNormals;
		arch << mesh.m_NumTangents;
		arch << mesh.m_NumUVs;
		arch << mesh.m_MaterialID;
		arch << mesh.m_MaxCorner;
		arch << mesh.m_MinCorner;

		if (mesh.m_NumIndices != 0) {
			arch.ArrayIn(mesh.m_IndexData, mesh.m_NumIndices);
		}

		if (mesh.m_NumVertices != 0) {
			arch.ArrayIn(mesh.m_VertexData, mesh.m_NumVertices);
		}

		if (mesh.m_NumNormals != 0) {
			arch.ArrayIn(mesh.m_NormalData, mesh.m_NumNormals);
		}

		if (mesh.m_NumTangents != 0) {
			arch.ArrayIn(mesh.m_TangentsData, mesh.m_NumTangents);
		}

		if (mesh.m_NumUVs != 0) {
			arch.ArrayIn(mesh.m_UVData, mesh.m_NumUVs);
		}

		return arch;
	}
	
}