#pragma once

#include <Math/kMath.hpp>
#include <KTL/DynArray.hpp>
#include <Interface/IIODevice.h>
#include <KTL/Archive.hpp>

#define ARCHIVEEXTENSION ".k3darch";

namespace k3d
{
	enum ArchiveVersion
	{
		VER_2014_4 = 0x201404
	};

	struct ArchHeader
	{
		uint32 Version;
	};

	enum ArchiveChunkType
	{
		CHUNK_MESH,
		CHUNK_PHYSICPROXY,
		CHUNK_MATERIAL,
		CHUNK_SHADER,
		CHUNK_GLFX,
		CHUNK_TEXTURE,
		CHUNK_TERRIAN,
		CHUNK_JSON,
		CHUNK_UNKNOWN
	};

	struct ArchClassChunk {
		char    ClassName[64];
		uint32  ClassInternalSize;
	};

	struct ArchChunkTag
	{
		uint32 ChunkType;
		uint32 ChunkSize;
	};
}


#if 0
class kMemoryArchiveReader : public Archive {
public:
  explicit kMemoryArchiveReader(const kByte *data);
  virtual ~kMemoryArchiveReader();

private:
  kByte *m_DataArchive;
};


class kMemoryArchiveWriter : public Archive {
public:
  explicit kMemoryArchiveWriter(kByte *data, int dataLen);
  virtual ~kMemoryArchiveWriter();

  void WriteToFile(const char * fileName);

private:
  uint32 m_DataLength;
  kByte* m_DataArchive;
};
#endif
