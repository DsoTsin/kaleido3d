#include "Kaleido3D.h"
#include "MeshData.h"
#include "Archive.h"
#include <cstring>
#include <assert.h>

namespace k3d 
{

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

		m_IndexData = nullptr;
		m_P3N3T2Buffer = nullptr;

		m_PrimType = PrimType::TRIANGLES;
		m_VtxFmt = VtxFormat::PER_INSTANCE;

		memset(m_MeshName, 0, 96);
	}

	Mesh::~Mesh()
	{
		Release();
	}

	void Mesh::Release()
	{
		SAFERELEASEARRAY(m_IndexData);
		m_IsLoaded = false;
		m_NumIndices = 0;
		m_NumVertices = 0;

		m_IndexData = nullptr;
		m_P3N3T2Buffer = nullptr;

		m_PrimType = PrimType::TRIANGLES;
		m_VtxFmt = VtxFormat::PER_INSTANCE;
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

	void Mesh::SetIndexBuffer(std::vector<uint32> &indexBuffer)
	{
		m_NumIndices = (uint32)indexBuffer.size();
		if (m_NumIndices != 0) {
			m_IndexData = new uint32[m_NumIndices];
			std::memcpy(m_IndexData, (uint32*)&std::move(indexBuffer)[0], m_NumIndices*sizeof(uint32));
		}
	}

	void Mesh::SetVertexBuffer(void *dataPtr) {
		assert(m_NumVertices!=0);
		switch (m_VtxFmt) {
		case VtxFormat::POS3_F32:
			m_P3Buffer = new Vertex3F[m_NumVertices];
			std::memcpy(m_P3Buffer, dataPtr, m_NumVertices*sizeof(Vertex3F));
			break;
		case VtxFormat::POS3_F32_NOR3_F32:
			m_P3N3Buffer = new Vertex3F3F[m_NumVertices];
			std::memcpy(m_P3N3Buffer, dataPtr, m_NumVertices*sizeof(Vertex3F3F));
			break;
		case VtxFormat::POS3_F32_NOR3_F32_UV2_F32:
			m_P3N3T2Buffer = new Vertex3F3F2F[m_NumVertices];
			std::memcpy(m_P3N3T2Buffer, dataPtr, m_NumVertices*sizeof(Vertex3F3F2F));
			break;
		default:
			break;
		}
	}
			
	Archive & operator >> (Archive &arch, Mesh &mesh)
	{
		//  arch.ArrayIn(Mesh::ClassName(), 64);
		arch.ArrayOut(mesh.m_MeshName, 96);
		
		arch >> mesh.m_VtxFmt;
		arch >> mesh.m_PrimType;

		arch >> mesh.m_NumIndices;
		arch >> mesh.m_NumVertices;
		
		arch >> mesh.m_MaterialID;
		arch >> mesh.m_MaxCorner;
		arch >> mesh.m_MinCorner;

		// IndexBuffer
		if (mesh.m_NumIndices != 0) {
			mesh.m_IndexData = new uint32[mesh.m_NumIndices];
			arch.ArrayOut(mesh.m_IndexData, mesh.m_NumIndices);
		}

		// VertexBuffer
		if (mesh.m_NumVertices != 0) {
			switch (mesh.m_VtxFmt) {
			case VtxFormat::POS3_F32_NOR3_F32_UV2_F32:
				mesh.m_P3N3T2Buffer = new Vertex3F3F2F[mesh.m_NumVertices];
				arch.ArrayOut<Vertex3F3F2F>(mesh.m_P3N3T2Buffer, mesh.m_NumVertices);
				break;
			case VtxFormat::POS3_F32_NOR3_F32:
				mesh.m_P3N3Buffer = new Vertex3F3F[mesh.m_NumVertices];
				arch.ArrayOut<Vertex3F3F>(mesh.m_P3N3Buffer, mesh.m_NumVertices);			
				break;
			case VtxFormat::POS3_F32:
				mesh.m_P3Buffer = new Vertex3F[mesh.m_NumVertices];
				arch.ArrayOut<Vertex3F>(mesh.m_P3Buffer, mesh.m_NumVertices);
				break;
			case VtxFormat::POS4_F32:
				mesh.m_P4Buffer = new Vertex4F[mesh.m_NumVertices];
				arch.ArrayOut<Vertex4F>(mesh.m_P4Buffer, mesh.m_NumVertices);
				break;
			default:
				break;
			}
		}

		return arch;
	}

	Archive & operator <<(Archive &arch, const Mesh &mesh)
	{
		arch.ArrayIn(Mesh::ClassName(), 64);
		arch.ArrayIn(mesh.m_MeshName, 96);

		arch << mesh.m_VtxFmt;
		arch << mesh.m_PrimType;

		arch << mesh.m_NumIndices;
		arch << mesh.m_NumVertices;

		arch << mesh.m_MaterialID;
		arch << mesh.m_MaxCorner;
		arch << mesh.m_MinCorner;

		if (mesh.m_NumIndices != 0) {
			arch.ArrayIn(mesh.m_IndexData, mesh.m_NumIndices);
		}

		// VertexBuffer
		if (mesh.m_NumVertices != 0) {
			switch (mesh.m_VtxFmt) {
			case VtxFormat::POS3_F32_NOR3_F32_UV2_F32:
				arch.ArrayIn<Vertex3F3F2F>(mesh.m_P3N3T2Buffer, mesh.m_NumVertices);
				break;
			case VtxFormat::POS3_F32_NOR3_F32:
				arch.ArrayIn<Vertex3F3F>(mesh.m_P3N3Buffer, mesh.m_NumVertices);
				break;
			case VtxFormat::POS3_F32:
				arch.ArrayIn<Vertex3F>(mesh.m_P3Buffer, mesh.m_NumVertices);
				break;
			case VtxFormat::POS4_F32:
				arch.ArrayIn<Vertex4F>(mesh.m_P4Buffer, mesh.m_NumVertices);
				break;
			default:
				break;
			}
		}

		return arch;
	}
	
}