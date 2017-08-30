#pragma once
#include "MeshData.h"

namespace k3d
{
	struct KALIGN(4) Vertex3F3F2F4F4I
	{
		float PosX, PosY, PosZ;
		float NorX, NorY, NorZ;
		float U, V;

		// Max. four bones per vertex
		float BoneWeights[4];
		uint32 BoneIDs[4];
	};

	class K3D_CORE_API RiggedMeshData
	{
	public:
		RiggedMeshData();
		~RiggedMeshData();

	private:

		bool                    m_IsLoaded;
		char                    m_MeshName[96];
				
		// IndexBuffer
		PrimType				m_PrimType;
		uint32					m_NumIndices;
		uint32*					m_IndexData;

		// VertexBuffer
		VtxFormat				m_VtxFmt;
		uint32					m_NumVertices;
		Vertex3F3F2F4F4I*		m_RiggedVertexBuffer;
	
		uint32			        m_MaterialID;
		kMath::Vec3f	        m_MaxCorner;
		kMath::Vec3f	        m_MinCorner;
	};
}