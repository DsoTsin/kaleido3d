#include "Archive.h"


#if 0
kMemoryArchiveReader::kMemoryArchiveReader(const kByte *data)
{

}

kMemoryArchiveReader::~kMemoryArchiveReader()
{

}


kMemoryArchiveWriter::kMemoryArchiveWriter(kByte *data, int dataLen)
  : m_DataArchive(nullptr)
  , m_DataLength(0)
{
  m_DataArchive = data;
  m_DataLength = dataLen;
}

kMemoryArchiveWriter::~kMemoryArchiveWriter()
{
  if(m_DataArchive) {
    ::free(data);
    m_DataArchive = nullptr;
    m_DataLength = 0;
  }
}

void kMemoryArchiveWriter::WriteToFile(const char *fileName)
{
  assert( fileName && m_DataArchive && m_DataLength > 0);
}
#endif
