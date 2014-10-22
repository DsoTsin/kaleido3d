#ifndef __k3dMesh_h__
#define __k3dMesh_h__
#pragma once

#include <Math/kMath.hpp>
#include <Math/kGeometry.hpp>

namespace Mesh{

  enum MVersion{
    VERSION_1_0 = 201402u,
    VERSION_1_1 = 201403u
  };

  enum Atrrib{
    NONE = 0x0,
    HAS_TRANSFORM = 0x1,
    HAS_BBOX = 0x1 << 1,
    HAS_TANGENT = 0x1 << 2,
    hAS_TRISTRIP = 0x1 << 3
  };

  /**
   *  Mesh Header <<POD>>
   */
  struct MeshHeader{
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
  struct MetaChunk  {
    char            Name[ 64 ];
    int             Lod;
    uint8_t         Attrib;
    char            MaterialName[ 64 ];
    float           BBox[ 6 ];
    uint32_t        NumTriangles;
    uint32_t        NumVertices;
    uint32_t        NumNormals;
    uint32_t        NumUVs;
  };

  struct KALIGN(4) Vertex
  {
    float x, y, z;
  };

  struct KALIGN(4) Normal
  {
    float x, y, z;
  };

  struct KALIGN(4) TexCoord
  {
    float u, v;
  };
}

class k3dMesh {
public:

  k3dMesh();
  ~k3dMesh();

  void    Release();

  void    SetMeshName(const char* meshName);
  void    SetBBox(float maxCorner[3], float minCorner[3]);
  void    SetIndexBuffer(std::vector<int> & indexBuffer);
  void    SetVertexBuffer(std::vector<kMath::Vec3f> & vertexBuffer);
  void    SetNormalBuffer(std::vector<kMath::Vec3f> & normalBuffer);
	void		SetUVBuffer(std::vector<kMath::Vec2f> & uvBuffer);
	void		SetMaterialID(uint32 matID);

  const char * MeshName() const;

  bool    IsLoaded() const;
  bool    HasNormalData() const;
  bool    HasTangentData() const;

  kMath::AABB GetBoundingBox() const;
  int     GetVertexNum() const;
  int     GetIndexNum() const;
  int     GetTrianglesNum() const;
  uint32  GetMaterialID() const;

	float * GetTangentBuffer() const;
	float * GetTexCoordBuffer() const;
	float * GetNormalBuffer() const;
  float * GetVertexBuffer() const;
  uint32* GetIndexBuffer() const;

  KOBJECT_CLASSNAME(k3dMesh)

  friend class k3dAssetManager;

  friend class k3dArchive& operator << (class k3dArchive & arch, const k3dMesh & mesh );
  friend class k3dArchive& operator >> (class k3dArchive & arch, k3dMesh & mesh );

private:
  k3dMesh(const k3dMesh &);
  k3dMesh& operator = (const k3dMesh &);


  bool                m_IsLoaded;

  char                m_MeshName[96];
  uint32              m_NumIndices;
  uint32              m_NumVertices;
  uint32              m_NumNormals;
  uint32              m_NumTangents;
  uint32              m_NumUVs;

  uint32*             m_IndexData;
  kMath::Vec3f*       m_VertexData;
  kMath::Vec3f*       m_NormalData;
  kMath::Vec3f*       m_TangentsData;
  kMath::Vec2f*       m_UVData;

  uint32        m_MaterialID;
  kMath::Vec3f  m_MaxCorner;
  kMath::Vec3f  m_MinCorner;

};


#endif
