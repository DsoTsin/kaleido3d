#include "k3dAssetManager.h"
#include <json/json.h>
#include <Core/k3dFile.h>
#include <Core/k3dLog.h>
#include <Core/k3dDataStream.h>
#include <Core/k3dVariant.h>
#include <Core/k3dMesh.h>
#include <Core/k3dImage.h>
#include <fstream>
#include <algorithm>

k3dDataStream & operator>>(k3dDataStream &data_stream, Mesh::MeshHeader &header)
{
  data_stream >> (uint32&)header.Version;
  return data_stream;
}

k3dDataStream & operator<<(k3dDataStream &data_stream, Mesh::MeshHeader header)
{
  data_stream << (uint32)header.Version;
  return data_stream;
}

k3dDataStream & operator>>(k3dDataStream &data_stream, Mesh::FixChunk &chunk)
{
  data_stream >> chunk.Tag >> chunk.DataType >> chunk.NumOfElement >> chunk.SzNextChunk;
  return data_stream;
}

k3dDataStream & operator<<(k3dDataStream &data_stream, Mesh::FixChunk chunk)
{
  data_stream << chunk.Tag << chunk.DataType << chunk.NumOfElement << chunk.SzNextChunk;
  return data_stream;
}

k3dDataStream & operator>>(k3dDataStream &data_stream, Mesh::MetaChunk &chunk)
{
  k3dVariant variant( (void*)chunk.Name, 64 );
  k3dVariant variant2( (void*)chunk.MaterialName, 64 );
  data_stream >> variant >> chunk.Lod >> (char&)chunk.Attrib >> variant2;

  for ( int i = 0; i < 6; i++ )
    data_stream >> chunk.BBox[ i ];

  data_stream >> chunk.NumTriangles >> chunk.NumVertices >> chunk.NumNormals >> chunk.NumUVs;

  return data_stream;
}

k3dDataStream & operator<<(k3dDataStream &data_stream, Mesh::MetaChunk chunk)
{
  k3dVariant variant( (void*)chunk.Name, 64 );
  k3dVariant variant2( (void*)chunk.MaterialName, 64 );
  data_stream << variant << chunk.Lod << (char)chunk.Attrib << variant2;

  for ( int i = 0; i < 6; i++ )
    data_stream << chunk.BBox[ i ];

  data_stream << chunk.NumTriangles << chunk.NumVertices << chunk.NumNormals << chunk.NumUVs;

  return data_stream;
}

//class kMeshLoader {
//public:
//  static bool Load(const char * meshName, k3dMesh::kObjPtr meshPtr) {
//    k3dMemMapFile meshFile;
//    if( !meshFile.Open(meshName, IORead) ) return false;
//    return true;
//  }
//};

k3dAssetManager::k3dAssetManager() : m_pThreadPool( NULL )
{
  m_IsLoading         = false;
  m_HasPendingObject  = false;
  m_NumPendingObject  = 0;
}

void k3dAssetManager::Init()
{
  if ( m_pThreadPool == NULL )
    m_pThreadPool = new std::thread_pool( 2 );

  LOG_MESSAGE("k3dAssetManager Initialized With two workers.");
}

void k3dAssetManager::Shutdown()
{
  if ( m_pThreadPool )
  {
    delete m_pThreadPool;
    m_pThreadPool = NULL;
  }
  LOG_MESSAGE("k3dAssetManager Shutdown.");
}

void k3dAssetManager::LoadAssetDescFile(const char *fileName)
{
  std::ifstream ifs(fileName);
  if(!ifs.good())
  {
    k3dLog::Fatal("k3dAssetManager::LoadAssetDescFile failed. Cannot find file %s.", fileName);
    return ;
  }
  Json::Value assetTable;
  Json::Reader asReader;
  if(!asReader.parse(ifs, assetTable, false))
  {
    k3dLog::Error("k3dAssetManager::LoadAssetDescFile Failed. Cannot Parse File %s.", fileName);
    ifs.close();
    return ;
  }
  ifs.close();

}

void k3dAssetManager::AddSearchPath( const char *path )
{
  typedef std::vector<k3dString>::const_iterator VSCIter;
  VSCIter pos = std::find( m_SearchPaths.begin(), m_SearchPaths.end(), k3dString( path ) );
  if ( pos == m_SearchPaths.end() )
  {
    m_SearchPaths.push_back( k3dString( path ) );
  }
}

void k3dAssetManager::CommitAsynResourceTask(
    const char *fileName,
    BytesPackage &bp,
    std::semaphore &sp,
    std::function<void ()> callback)
{
  auto result = m_pThreadPool->enqueue(
        [&bp, &sp](const char *name, std::function<void()> _callback)
  {
//    ++m_NumPendingObject;
    k3dFile file;
    bool opened = file.Open( name, IORead );
    assert(opened == true);
    if(opened)
    {
      int64 bytes_length = file.GetSize();
      bp.Bytes.assign(bytes_length+1, '\0');
      //bp.Bytes.assign()
      file.Read( (char*)&bp.Bytes[0], bytes_length );
      file.Close();

      _callback();
    }
    else
    {
      k3dLog::Error("k3dAssetManager::CommitAsynResourceTask failed. Cannot find file %s.", name);
    }
    sp.notify();

//    --m_NumPendingObject;
  },
  fileName,
  callback
  );
}

void k3dAssetManager::CommitAsynResourceTask(const char *fileName, BytesPackage &bp, std::semaphore &sp)
{
  auto result = m_pThreadPool->enqueue(
        [&bp, &sp] ( const char *name )
  {
//    ++m_NumPendingObject;
    k3dMemMapFile file;
    bool opened = file.Open( name, IORead );
    if (opened)
    {
      int bytes_length = (int)file.GetSize();
      bp.Bytes.reserve(bytes_length);
      file.Read( (char*)&bp.Bytes[0], bytes_length );
      file.Close();
    }
    else
    {
      k3dLog::Error("k3dAssetManager::CommitSynResourceTask failed. Cannot find file %s.", name);
    }
    sp.notify();
//    --m_NumPendingObject;
  },
  fileName
  );
}

void k3dAssetManager::CommitAsynResourceTask(const char *fileName, BytesPackage &bp, std::atomic<bool> &finished)
{
  auto result = m_pThreadPool->enqueue(
        [&bp, &finished] ( const char *name )
  {
//    ++m_NumPendingObject;
    k3dMemMapFile file;
    bool opened = file.Open( name, IORead );
    if (opened)
    {
      int bytes_length = (int)file.GetSize();
      bp.Bytes.reserve(bytes_length);
      file.Read( (char*)&bp.Bytes[0], bytes_length );
      file.Close();
    }
    else
    {
      k3dLog::Error("k3dAssetManager::CommitSynResourceTask failed. Cannot find file %s.", name);
    }
    finished.store(true, std::memory_order_release);

//    --m_NumPendingObject;
  },
  fileName
  );
}

void k3dAssetManager::CommitSynResourceTask(const char *fileName, BytesPackage &bp)
{
  k3dFile file;
  if(file.Open(fileName, IORead))
  {
    int64 length = file.GetSize();
    bp.Bytes.resize(length);
    int64 __read = file.Read( (char*)&bp.Bytes[0], length);
    assert(__read == length);
    file.Close();
  }
}


void k3dAssetManager::Free( char *byte_ptr )
{
  ::free( byte_ptr );
}

std::shared_ptr<k3dMesh> k3dAssetManager::FindMesh(const char *meshName)
{
  MapMeshIter iter = m_MeshMap.find(k3dString(meshName));
  if( iter == m_MeshMap.end() )
    return std::shared_ptr<k3dMesh>();
  return (iter->second);
}

std::shared_ptr<k3dImage> k3dAssetManager::FindImage(const char *imgName)
{
  MapImageIter iter = m_ImageMap.find(k3dString(imgName));
  if( iter == m_ImageMap.end() )
    return std::shared_ptr<k3dImage>();
  return (iter->second);
}

