/********************************************************
k3dFile (IO)
FileStdio, FileMemoryMap, FileArchive<zipped>
*********************************************************/
#pragma once
#ifndef __k3dFile_h__
#define __k3dFile_h__

#include <IIODevice.h>
#include <Config/OSHeaders.h>

class k3dFile : public IIODevice
{
public:
  k3dFile();
  explicit k3dFile( const char *fileName );
  ~k3dFile();
  
  bool      Open( IOFlag flag );
  bool      Open( const char* fileName, IOFlag flag );

  int64     GetSize();

  bool      IsEOF();

  size_t    Read(char *ptr, size_t len );
  size_t    Write( void *ptr, size_t len );
  
  bool      Seek( size_t offset );
  bool      Skip( size_t offset );

  void      Flush();
  void      Close();

  static k3dFile * CreateIOInterface();

private:
#ifdef K3DPLATFORM_OS_WIN
  HANDLE      m_hFile;
#else
  int         m_fd;
#endif
  bool        m_EOF;
  int64       m_CurOffset;
  const char *m_pFileName;
};

KTYPE_ISTYPE_TEMPLATE( k3dFile, IIODevice );

class k3dMemMapFile : public IIODevice
{
public:
  k3dMemMapFile();
  ~k3dMemMapFile();

  int64     GetSize();
//---------------------------------------------------------
  bool      Open( const char* fileName, IOFlag mode );
  size_t    Read( char * data_ptr, size_t len );
  size_t    Write(void *, size_t );
  bool      Seek( size_t offset );
  bool      Skip( size_t offset );
  bool      IsEOF();
  void      Flush();
  void      Close();
//---------------------------------------------------------

//---------------------------------------------------------
  /// FileData
  /// \brief FileData
  /// \return data const pointer
  const kByte* FileData() { return m_pData; }

  template <class T>
  /// Convert FileBlocks To Class
  /// \brief ConvertToClass
  /// \param address_of_file
  /// \param object_offset
  /// \return Object Pointer
  static T*   ConvertToClass(char * &address_of_file, uint32 object_offset)
  {
    char *objectptr = address_of_file+object_offset;
    address_of_file += sizeof(T) + object_offset;
    return static_cast<T*>(objectptr);
  }
//---------------------------------------------------------

  /// General Interface For GetIODevice
  /// \brief CreateIOInterface
  /// \return An IIODevice Pointer
  static k3dMemMapFile* CreateIOInterface();
private:

#ifdef K3DPLATFORM_OS_WIN
  HANDLE    m_FileHandle;
  HANDLE    m_FileMappingHandle;
#elif defined(K3DPLATFORM_OS_LINUX) || defined(K3DPLATFORM_OS_MAC)
  int       m_Fd;
#endif
  size_t    m_szFile;
  kByte *   m_pData;
  kByte *   m_pCur;
};

KTYPE_ISTYPE_TEMPLATE( k3dMemMapFile, IIODevice );

#endif
