#include "Kaleido3D.h"
#include "RiggedMeshData.h"

namespace k3d
{
	RiggedMeshData::RiggedMeshData()
		: m_IsLoaded(false)
		, m_NumIndices(0)
		, m_IndexData(nullptr)
		, m_NumVertices(0)
		, m_RiggedVertexBuffer(nullptr)
	{

	}

	RiggedMeshData::~RiggedMeshData()
	{
		
	}
}