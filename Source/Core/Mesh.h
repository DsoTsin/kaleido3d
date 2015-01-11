#ifndef __k3dMesh_h__
#define __k3dMesh_h__
#pragma once

#include <Math/kMath.hpp>
#include <Math/kGeometry.hpp>
#include <Interface/IReflectable.h>

namespace kMath {
	typedef tVectorN<float, 2> Vec2f;
	typedef tVectorN<float, 3> Vec3f;
	typedef tVectorN<float, 4> Vec4f;
}

namespace k3d {

	enum MVersion {
		VERSION_1_0 = 201402u,
		VERSION_1_1 = 201501u
	};

	enum Atrrib {
		NONE = 0x0,
		HAS_TRANSFORM = 0x1,
		HAS_BBOX = 0x1 << 1,
		HAS_TANGENT = 0x1 << 2,
		HAS_TRISTRIP = 0x1 << 3
	};

	/**
	*  Mesh Header <<POD>>
	*/
	struct MeshHeader {
		MVersion     Version;
	};

	enum ChunkTag
	{
		META_DATA,
		POSITION_DATA,
		NORMAL_DATA,
		TEXCOORD_DATA,
		BINORMAL_DATA,
		TANGENT_DATA,
		BINTANGENT_DATA,
		PARTICLE_DATA,
		ELEMENTINDEX_DATA,
		TRISTRIP_INDEX_DATA,
		UNKNOWN
	};

	enum ChunkDataType
	{
		FLOAT32,
		UINT8,
		INT32,
		UINT32,
		DOUBLE,
		OTHER
	};

	// FixChunk
	// Members : Tag, DataType, NumOfEle, Offset
	struct FixChunk
	{
		uint32    Tag;
		uint32    DataType;
		uint32    NumOfElement;
		uint64    SzNextChunk;
	};

	// MeshChunk
	// Members: MeshName[64], Lod[int], Attrib[uint8], MaterialName[64]...
	//          BBox[float6], NumTri[uint32], NumVerts[u32], NumNors[u32], NumUVs[u32]
	struct MetaChunk {
		char            Name[64];
		int             Lod;
		uint8_t         Attrib;
		char            MaterialName[64];
		float           BBox[6];
		uint32_t        NumTriangles;
		uint32_t        NumVertices;
		uint32_t        NumNormals;
		uint32_t        NumUVs;
	};

	struct KALIGN(4) Vertex4F
	{
		float PosX, PosY, PosZ, PosW;
	};

	struct KALIGN(4) Vertex3F
	{
		float PosX, PosY, PosZ;
	};

	struct KALIGN(4) Vertex3F3F
	{
		float PosX, PosY, PosZ;
		float NorX, NorY, NorZ;
	};

	struct KALIGN(4) Vertex3F2F
	{
		float PosX, PosY, PosZ;
		float U, V;
	};

	struct KALIGN(4) Vertex3F3F2F
	{
		float PosX, PosY, PosZ;
		float NorX, NorY, NorZ;
		float U, V;
	};

	struct KALIGN(4) Normal3F
	{
		float x, y, z;
	};

	struct KALIGN(4) TexCoord2F
	{
		float u, v;
	};

	enum VtxFormat {
		POS3_F32,
		POS4_F32,
		POS3_F32_UV2_F32,
		POS3_F32_NOR3_F32,
		POS3_F32_NOR3_F32_UV2_F32,
		POS3_F32_NOR3_F32_UV2X2_F32,
		POS3_F32_NOR3_F32_UV2X3_F32,
		PER_INSTANCE // all components are seperated
	};

	enum PrimType {
		POINTS,
		TRIANGLES,
		TRIANGLE_STRIPS
	};

	class Mesh : public IReflectable {
	public:

		Mesh();
		~Mesh();

		void    Release();

		void    SetMeshName(const char* meshName);
		void    SetBBox(float maxCorner[3], float minCorner[3]) {
				m_MaxCorner.init(maxCorner);
				m_MinCorner.init(minCorner);
		}
		void	SetMaterialID(uint32 matID) { m_MaterialID = matID; }

		const char * MeshName() const {	return m_MeshName; }
		bool		IsLoaded() const { return m_IsLoaded; }

		kMath::AABB GetBoundingBox() const { return kMath::AABB(m_MaxCorner, m_MinCorner); }
		uint32		GetMaterialID() const { return m_MaterialID; }
		
		int			GetIndexNum() const	{ return m_NumIndices; }
		uint32*		GetIndexBuffer() const { return m_IndexData; }
		void		SetIndexBuffer(std::vector<uint32> & indexBuffer);

		PrimType	GetPrimType() const { return m_PrimType; }
		void		SetPrimType(PrimType const & type) { m_PrimType = type; }

		int			GetVertexNum() const { return m_NumVertices; }
		VtxFormat	GetVertexFormat() const { return m_VtxFmt; }
		void		SetVertexFormat(VtxFormat const & fmt) { m_VtxFmt = fmt; }

		float *		GetVertexBuffer() const { return (float*)&m_P3Buffer[0]; }
		void		SetVertexBuffer(void* dataPtr);
		void		SetVertexNum(int num) { m_NumVertices = num; }

		Mesh *		Reflect() override { return new Mesh; }

		KOBJECT_CLASSNAME(Mesh)

		friend class AssetManager;

		friend class Archive& operator << (class Archive & arch, const Mesh & mesh);
		friend class Archive& operator >> (class Archive & arch, Mesh & mesh);

	private:
		Mesh(const Mesh &) = delete;
		Mesh& operator = (const Mesh &) = delete;

		bool                m_IsLoaded;
		char                m_MeshName[96];
				
		// IndexBuffer
		PrimType				m_PrimType;
		uint32					m_NumIndices;
		uint32*					m_IndexData;

		// VertexBuffer
		VtxFormat				m_VtxFmt;
		uint32					m_NumVertices;
		union {
			Vertex3F3F2F*		m_P3N3T2Buffer;
			Vertex3F3F*			m_P3N3Buffer;
			Vertex3F*			m_P3Buffer;
			Vertex4F*			m_P4Buffer;
		};

		uint32			m_MaterialID;
		kMath::Vec3f	m_MaxCorner;
		kMath::Vec3f	m_MinCorner;
	};

	typedef std::shared_ptr<Mesh> SpMesh;
}

#endif
