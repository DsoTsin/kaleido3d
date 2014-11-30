#pragma once

#include <Math/kMath.hpp>
#include <Interface/IIODevice.h>

#define ARCHIVEEXTENSION ".k3darch";

namespace k3d {

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

	class Archive {
	public:
		Archive() : Handler(nullptr) {}
		virtual ~Archive() {}

		void SetIODevice(IIODevice * ioHandler) {
			Handler = ioHandler;
		}

		template <typename T>
		Archive & operator >> (T & data) {
			assert(std::is_pointer<T>::value != true && "cannot be serialize, not a pod class!!");
			Handler->Read((char*)&data, sizeof(T));
			return *this;
		}

		template <typename T>
		Archive & operator << (const T data) {
			assert(std::is_pointer<T>::value != true && "cannot be serialize, not a pod class!!");
			Handler->Write((kByte*)&data, sizeof(T));
			return *this;
		}

		template <typename T>
		void ArrayIn(T *dataArray, uint32 elemCount) {
			assert(std::is_pointer<T>::value != true && "ArrayIn Error: not a pod class");
			Handler->Write((kByte*)dataArray, elemCount*sizeof(T));
		}

		template <typename T>
		void ArrayOut(T *dataArray, uint32 elemCount) {
			assert(std::is_pointer<T>::value != true && "ArrayOut Error: not a pod class");
			Handler->Read((char*)dataArray, elemCount*sizeof(T));
		}

		virtual void FlushCurrentCache() {
			Handler->Flush();
		}

	protected:

		IIODevice* Handler;
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
