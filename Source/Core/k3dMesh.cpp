#include "k3dMesh.h"
#include <assert.h>
#include <Core/k3dArchive.h>

#define SAFERELEASEARRAY(x) \
  if(x) {\
  delete [] x;\
  x=nullptr;\
  }

k3dMesh::k3dMesh()
{
  m_IsLoaded      = false;
  m_NumIndices    = 0;
  m_NumVertices   = 0;
  m_NumNormals    = 0;
  m_NumTangents   = 0;
  m_NumUVs        = 0;

  m_IndexData     = nullptr;
  m_VertexData    = nullptr;
  m_NormalData    = nullptr;
  m_TangentsData  = nullptr;
  m_UVData        = nullptr;
  memset(m_MeshName, 0, 96);
}

k3dMesh::~k3dMesh()
{
  Release();
}

void k3dMesh::Release()
{
  SAFERELEASEARRAY(m_IndexData);
  SAFERELEASEARRAY(m_VertexData);
  SAFERELEASEARRAY(m_NormalData);
  SAFERELEASEARRAY(m_TangentsData);
  SAFERELEASEARRAY(m_UVData);
  m_IsLoaded      = false;
  m_NumIndices    = 0;
  m_NumVertices   = 0;
  m_NumNormals    = 0;
  m_NumTangents   = 0;
  m_NumUVs        = 0;
}

void k3dMesh::SetMeshName(const char *meshName)
{
  assert(meshName && "MeshName cannot be nullptr");
  const char * ptr = meshName;
  int pos = 0;
  while(*ptr!=0 && pos<96){
    m_MeshName[pos] = *ptr;
    ptr++; pos++;
  }
}

void k3dMesh::SetBBox(float maxCorner[], float minCorner[])
{
  m_MaxCorner.init(maxCorner);
  m_MinCorner.init(minCorner);
}

void k3dMesh::SetIndexBuffer(std::vector<int> &indexBuffer)
{
  m_NumIndices = (uint32)indexBuffer.size();
  if(m_NumIndices!=0) {
    m_IndexData = new uint32 [m_NumIndices];
    memcpy(m_IndexData, (uint32*)&std::move(indexBuffer)[0], m_NumIndices*sizeof(uint32));
//    memc
  }
}

void k3dMesh::SetVertexBuffer(std::vector<kMath::Vec3f> &vertexBuffer)
{
  m_NumVertices = (uint32)vertexBuffer.size();
  if(m_NumVertices!=0) {
    m_VertexData = new kMath::Vec3f [m_NumVertices];
    memcpy( m_VertexData, (kMath::Vec3f*)&std::move(vertexBuffer)[0], m_NumVertices*sizeof(kMath::Vec3f) );
  }
}

void k3dMesh::SetNormalBuffer(std::vector<kMath::Vec3f> &normalBuffer)
{
  m_NumNormals = (uint32)normalBuffer.size();
  if(m_NumNormals!=0) {
    m_NormalData = new kMath::Vec3f[m_NumIndices];
    memcpy( m_NormalData, (kMath::Vec3f*)&std::move(normalBuffer)[0], m_NumNormals*sizeof(kMath::Vec3f) );
  }
}

void k3dMesh::SetUVBuffer(std::vector<kMath::Vec2f> & uvBuffer)
{
	m_NumUVs = (uint32)uvBuffer.size();
	if (m_NumNormals != 0) {
		m_UVData = new kMath::Vec2f[m_NumUVs];
		memcpy(m_UVData, (kMath::Vec2f*)&std::move(uvBuffer)[0], m_NumUVs*sizeof(kMath::Vec2f));
	}
}

void k3dMesh::SetMaterialID(uint32 matID)
{
	m_MaterialID = matID;
}

bool k3dMesh::IsLoaded() const
{
  return m_IsLoaded;
}

bool k3dMesh::HasNormalData() const
{
  return (m_NumNormals!=0);
}

bool k3dMesh::HasTangentData() const
{
  return (m_NumTangents!=0);
}

kMath::AABB k3dMesh::GetBoundingBox() const
{
  return kMath::AABB(m_MaxCorner, m_MinCorner);
}

int k3dMesh::GetVertexNum() const
{
  return m_NumVertices;
}

int k3dMesh::GetIndexNum() const
{
  return m_NumIndices;
}

int k3dMesh::GetTrianglesNum() const
{
  return m_NumIndices/3;
}

uint32 k3dMesh::GetMaterialID() const
{
  return m_MaterialID;
}

float * k3dMesh::GetTangentBuffer() const
{
	assert(m_TangentsData && "m_TangentData == nullptr !");
	return (float*)&m_TangentsData[0];
}

float * k3dMesh::GetTexCoordBuffer() const
{
	assert(m_UVData && "m_UVData == nullptr !");
	return (float*)&m_UVData[0];
}

float * k3dMesh::GetNormalBuffer() const
{
	assert(m_NormalData && "m_NormalData == nullptr !");
	return (float*)&m_NormalData[0];
}

float *k3dMesh::GetVertexBuffer() const
{
  assert(m_VertexData && "m_VertexData == nullptr!!");
  return (float*)&m_VertexData[0];
}

uint32 *k3dMesh::GetIndexBuffer() const
{
  assert(m_IndexData && "m_IndexData == nullptr!!");
  return m_IndexData;
}

const char * k3dMesh::MeshName() const
{
	return m_MeshName;
}

k3dArchive & operator >>(k3dArchive &arch, k3dMesh &mesh)
{
//  arch.ArrayIn(k3dMesh::ClassName(), 64);
  arch.ArrayOut(mesh.m_MeshName, 96);
  arch >> mesh.m_NumIndices;
  arch >> mesh.m_NumVertices;
  arch >> mesh.m_NumNormals;
  arch >> mesh.m_NumTangents;
  arch >> mesh.m_NumUVs;
  arch >> mesh.m_MaterialID;
  arch >> mesh.m_MaxCorner;
  arch >> mesh.m_MinCorner;

  if(mesh.m_NumIndices!=0) {
    mesh.m_IndexData = new uint32 [mesh.m_NumIndices];
    arch.ArrayOut(mesh.m_IndexData, mesh.m_NumIndices);
  }

  if(mesh.m_NumVertices!=0) {
    mesh.m_VertexData = new kMath::Vec3f [mesh.m_NumVertices];
    arch.ArrayOut(mesh.m_VertexData, mesh.m_NumVertices);
  }

  if(mesh.m_NumNormals!=0) {
    mesh.m_NormalData = new kMath::Vec3f [mesh.m_NumNormals];
    arch.ArrayOut(mesh.m_NormalData, mesh.m_NumNormals);
  }

  if(mesh.m_NumTangents!=0) {
    mesh.m_TangentsData = new kMath::Vec3f [mesh.m_NumTangents];
    arch.ArrayOut(mesh.m_TangentsData, mesh.m_NumTangents);
  }

  if(mesh.m_NumUVs!=0) {
    mesh.m_UVData = new kMath::Vec2f [mesh.m_NumUVs];
    arch.ArrayOut(mesh.m_UVData, mesh.m_NumUVs);
  }

  return arch;
}

k3dArchive & operator <<(k3dArchive &arch, const k3dMesh &mesh)
{
  arch.ArrayIn(k3dMesh::ClassName(), 64);
  arch.ArrayIn(mesh.m_MeshName, 96);
  arch << mesh.m_NumIndices;
  arch << mesh.m_NumVertices;
  arch << mesh.m_NumNormals;
  arch << mesh.m_NumTangents;
  arch << mesh.m_NumUVs;
  arch << mesh.m_MaterialID;
  arch << mesh.m_MaxCorner;
  arch << mesh.m_MinCorner;

  if(mesh.m_NumIndices!=0) {
    arch.ArrayIn(mesh.m_IndexData, mesh.m_NumIndices);
  }

  if(mesh.m_NumVertices!=0) {
    arch.ArrayIn(mesh.m_VertexData, mesh.m_NumVertices);
  }

  if(mesh.m_NumNormals!=0) {
    arch.ArrayIn(mesh.m_NormalData, mesh.m_NumNormals);
  }

  if(mesh.m_NumTangents!=0) {
    arch.ArrayIn(mesh.m_TangentsData, mesh.m_NumTangents);
  }

  if(mesh.m_NumUVs!=0) {
    arch.ArrayIn(mesh.m_UVData, mesh.m_NumUVs);
  }

  return arch;
}
