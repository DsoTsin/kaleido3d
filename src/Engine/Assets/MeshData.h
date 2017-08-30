#ifndef __k3dMesh_h__
#define __k3dMesh_h__
#pragma once

#include <Core/Math/kMath.hpp>
#include <Core/Math/kGeometry.hpp>
#include <Core/Interface/IMesh.h>
#include <Core/KTL/SharedPtr.hpp>
#include <sstream>

#include "Bundle.h"

namespace kMath 
{
	typedef tVectorN<float, 2> Vec2f;
	typedef tVectorN<float, 3> Vec3f;
	typedef tVectorN<float, 4> Vec4f;
}

namespace k3d 
{
	enum class EMeshVersion : uint64
	{
		VERSION_1_0 = 201402u,
		VERSION_1_1 = 201501u
	};
	
	/**
	*  Mesh Header <<POD>>
	*/
	struct MeshHeader {
		EMeshVersion     Version;
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


	class K3D_CORE_API MeshData : public IMesh {
	public:

		MeshData();
		~MeshData();

		void    Release();

		void    SetMeshName(const char* meshName);
		void    SetBBox(float maxCorner[3], float minCorner[3]) {
				m_MaxCorner.init(maxCorner);
				m_MinCorner.init(minCorner);
		}
		void	SetMaterialID(uint32 matID) { m_MaterialID = matID; }

		const char * Name() const {	return m_MeshName; }
		bool		IsLoaded() const { return m_IsLoaded; }

		kMath::AABB GetBoundingBox() const override{ return kMath::AABB(m_MaxCorner, m_MinCorner); }
		uint32		GetMaterialID() const override { return m_MaterialID; }
		
		int			GetIndexNum() const override	{ return m_NumIndices; }
		uint32*		GetIndexBuffer() const override { return m_IndexData; }
		void		SetIndexBuffer(std::vector<uint32> & indexBuffer);

		PrimType	GetPrimType() const override { return m_PrimType; }
		void		SetPrimType(PrimType const & type) { m_PrimType = type; }

		int			GetVertexNum() const override { return m_NumVertices; }
		VtxFormat	GetVertexFormat() const override { return m_VtxFmt; }
		void		SetVertexFormat(VtxFormat const & fmt) { m_VtxFmt = fmt; }

		float *		GetVertexBuffer() const override { return (float*)&m_P3Buffer[0]; }
		void		SetVertexBuffer(void* dataPtr);
		void		SetVertexNum(int num) { m_NumVertices = num; }

		std::string DumpMeshInfo() 
		{
			std::ostringstream meshInfo;
			meshInfo << "[Mesh]\n"
				<< "\tName:" << this->Name()
				<< "\n\tFormat:" << VtxFormatToString(this->GetVertexFormat())
				<< "\n\tPrimitiveType:" << PrimTypeToString(this->GetPrimType())
				<< "\n\tNumVertices:" << this->GetVertexNum()
				<< "\n\tNumIndices:" << this->GetIndexNum()
				<< "\n";
			return meshInfo.str();
		}

		KOBJECT_CLASSNAME(MeshData)

		friend class AssetManager;

		friend K3D_CORE_API class Archive& operator << (class Archive & arch, const MeshData & mesh);
		friend K3D_CORE_API class Archive& operator >> (class Archive & arch, MeshData & mesh);

	public:

		static uint32	GetVertexByteWidth(VtxFormat format, uint32 vertexNum)
		{
			static uint32 elementByteWidth[] = {
				sizeof(Vertex3F),
				sizeof(Vertex4F),
				sizeof(Vertex3F2F),
				sizeof(Vertex3F3F),
				sizeof(Vertex3F3F2F)
			};
			if (format > VtxFormat::POS3_F32_NOR3_F32_UV2_F32)
				return 0;
			return elementByteWidth[(uint32)format] * vertexNum;
		}

		static uint32	GetVertexStride(VtxFormat format)
		{
			static uint32 elementByteStride[] = {
				sizeof(Vertex3F),
				sizeof(Vertex4F),
				sizeof(Vertex3F2F),
				sizeof(Vertex3F3F),
				sizeof(Vertex3F3F2F)
			};
			if (format > VtxFormat::POS3_F32_NOR3_F32_UV2_F32)
				return 0;
			return elementByteStride[(uint32)format];
		}

		static std::string VtxFormatToString(VtxFormat format)
		{
			static std::string vtxFormatStr[] = {
				"Vertex3F",
				"Vertex4F",
				"Vertex3F2F",
				"Vertex3F3F",
				"Vertex3F3F2F"
			};
			if (format > VtxFormat::POS3_F32_NOR3_F32_UV2_F32)
				return "Unknown";
			return vtxFormatStr[(uint32)format];
		}

		static std::string PrimTypeToString(PrimType primType)
		{
			static std::string primTypeStr[] = {
				"POINTS",
				"TRIANGLES",
				"TRIANGLE_STRIPS"
			};
			return primTypeStr[(uint32)primType];
		}

	private:
		MeshData(const MeshData &) = delete;
		MeshData& operator = (const MeshData &) = delete;

		bool                    m_IsLoaded;
		char                    m_MeshName[96];
				
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

		uint32			        m_MaterialID;
		kMath::Vec3f	        m_MaxCorner;
		kMath::Vec3f	        m_MinCorner;
	};

	typedef SharedPtr<MeshData> SpMesh;
}

#endif
