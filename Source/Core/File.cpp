#include "Kaleido3D.h"
#include "File.h"
#include <assert.h>
#include "LogUtil.h"
#ifdef min
#undef min
#endif
#include <algorithm>

#include <Config/OSHeaders.h>
#include "Utils/StringUtils.h"

namespace k3d {

	File::File(const char *fileName)
		:
#if K3DPLATFORM_OS_WIN
		m_hFile(NULL),
#else
		m_fd(-1),
#endif
		 m_EOF(false)
		, m_CurOffset(0)
		, m_pFileName(fileName)
	{}

	File::File()
		:
#if K3DPLATFORM_OS_WIN
		m_hFile(NULL),
#else
		m_fd(-1),
#endif
		 m_EOF(false)
		, m_CurOffset(0)
		, m_pFileName(NULL)
	{}

	File::~File() { Close(); }

	bool File::Open(IOFlag flag)
	{
		return Open(m_pFileName, flag);
	}

	bool File::Open(const char *fileName, IOFlag flag)
	{
#if K3DPLATFORM_OS_WIN
		wchar_t name_buf[1024];
		StringUtil::CharToWchar(fileName, name_buf, sizeof(name_buf));

		DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		int   accessRights = 0;

		if (flag & IORead)
			accessRights |= GENERIC_READ;
		if (flag & IOWrite)
			accessRights |= GENERIC_WRITE;

		SECURITY_ATTRIBUTES securityAttrs = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		DWORD  createDisp = (flag & IOWrite) ? CREATE_ALWAYS : OPEN_EXISTING;

		m_hFile = ::CreateFileW(
			name_buf,// file to open
			accessRights,
			shareMode,
			&securityAttrs,
			createDisp,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return false;
#else
        m_fd = ::open(fileName, flag == IORead ? O_RDONLY : O_WRONLY);
		if (m_fd < 0) return false;
#endif
		return true;
	}


#if K3DPLATFORM_OS_WIN

	bool File::Open(const WCHAR *fileName, IOFlag flag)
	{
		DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		int   accessRights = 0;

		if (flag & IORead)
			accessRights |= GENERIC_READ;
		if (flag & IOWrite)
			accessRights |= GENERIC_WRITE;

		SECURITY_ATTRIBUTES securityAttrs = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		DWORD  createDisp = (flag & IOWrite) ? CREATE_ALWAYS : OPEN_EXISTING;

		m_hFile = ::CreateFileW(
			fileName,// file to open
			accessRights,
			shareMode,
			&securityAttrs,
			createDisp,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return false;
		return true;
	}

#endif


	int64 File::GetSize() {
		int64 len = 0;
#if K3DPLATFORM_OS_WIN
		len = ::GetFileSize(m_hFile, NULL);
#else
		struct stat st;
		if (fstat(m_fd, &st) != 0) return -1;
		len = st.st_size;
#endif
		return len;
	}

	bool File::IsEOF()
	{
		return m_EOF || m_CurOffset == GetSize();
	}

	size_t File::Read(char *data, size_t len)
	{
#if K3DPLATFORM_OS_WIN
		if (m_hFile == INVALID_HANDLE_VALUE)
			return size_t(-1);
		DWORD bytesToRead = (DWORD)len;
		static const DWORD maxBlockSize = 32 * (1 << 20);
		int64 totalRead = 0;
		do {
			DWORD blockSize = std::min(bytesToRead, maxBlockSize);
			DWORD bytesRead;
			if (!::ReadFile(m_hFile, data + totalRead, blockSize, &bytesRead, NULL))
			{
				if (totalRead == 0)
					//Error
					return size_t(-1);
				break;
			}
			if (bytesRead == 0)
				break;
			totalRead += bytesRead;
			bytesToRead -= bytesRead;
		} while (totalRead < (int64)len);

		return totalRead;
#else
		size_t _read = 0;
		_read = ::read(m_fd, data, len);
		return _read;
#endif
	}

	size_t File::Write(void *data, size_t len)
	{
		size_t written = 0;
#ifdef K3DPLATFORM_OS_WIN
		WriteFile(m_hFile, data, (DWORD)len, (LPDWORD)&written, NULL);
#else
		written = ::write(m_fd, data, len);
#endif

		m_CurOffset += written;
		return written;
	}

	bool File::Seek(size_t offset)
	{
#if K3DPLATFORM_OS_WIN
		m_CurOffset = ::SetFilePointer(m_hFile, (LONG)offset, NULL, 0);
#else
		m_CurOffset = ::lseek(m_fd, offset, SEEK_SET);
#endif
		return m_CurOffset >= 0;
	}

	bool File::Skip(size_t offset)
	{
#if K3DPLATFORM_OS_WIN
		m_CurOffset = ::SetFilePointer(m_hFile, (LONG)offset, NULL, 1);
#else
		m_CurOffset = ::lseek(m_fd, offset, SEEK_CUR);
#endif
		return m_CurOffset >= 0;
	}

	void File::Flush()
	{  }

	void File::Close()
	{
#ifdef K3DPLATFORM_OS_WIN
		if (m_hFile)
		{
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
#else
		if (m_fd) ::close(m_fd);
#endif
	}

	File * File::CreateIOInterface()
	{
		return new File;
	}
	//--------------------------------------------------------------------------------------------

	MemMapFile::MemMapFile() :
#if K3DPLATFORM_OS_WIN
		m_FileHandle(NULL), m_FileMappingHandle(NULL),
#else
		m_Fd(-1),
#endif
		m_szFile(0), m_pData(NULL)
	{
	}

	MemMapFile::~MemMapFile()
	{
		Close();
	}

	int64 MemMapFile::GetSize()
	{
		return m_szFile;
	}

	bool MemMapFile::Open(const char* fileName, IOFlag mode)
	{
		assert(mode == IORead);

#if K3DPLATFORM_OS_WIN
		wchar_t name_buf[1024];
		StringUtil::CharToWchar(fileName, name_buf, sizeof(name_buf));
		m_FileHandle = ::CreateFileW(name_buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (m_FileHandle == INVALID_HANDLE_VALUE) return false;

		m_szFile = ::GetFileSize(m_FileHandle, NULL);
		if (m_szFile == INVALID_FILE_SIZE) return false;

		m_FileMappingHandle = ::CreateFileMapping(m_FileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
		if (m_FileMappingHandle == INVALID_HANDLE_VALUE) return false;

		m_pData = (unsigned char*)MapViewOfFile(m_FileMappingHandle, FILE_MAP_READ, 0, 0, 0);
		if (m_pData == NULL) return false;
#else
		m_Fd = open(fileName, O_RDONLY);
		if (m_Fd == -1) return false;

		struct stat st;
		if (fstat(m_Fd, &st) != 0) return false;

		m_szFile = st.st_size;
		if (st.st_size == 0) return false;

		m_pData = (unsigned char*)mmap(NULL, m_szFile, PROT_READ, MAP_PRIVATE, m_Fd, 0);
		if (m_pData == MAP_FAILED) return false;
#endif
		m_pCur = m_pData;
		return true;
	}

	size_t MemMapFile::Read(char *data_ptr, size_t len)
	{
		size_t bytes_to_end = m_szFile - (m_pCur - m_pData);
		if (len <= bytes_to_end) {
			memcpy(data_ptr, m_pCur, len);
			m_pCur += len;
			return len;
		}
		memcpy(data_ptr, m_pCur, bytes_to_end);
		m_pCur += bytes_to_end;
		return bytes_to_end;
	}

	size_t MemMapFile::Write(void *, size_t)
	{
		assert(0 && "MemMapFile::Write(): can't write into the memory");
		return 0;
	}

	bool MemMapFile::Seek(size_t offset)
	{
		if (offset > m_szFile) return false;
		m_pCur = m_pData + offset;
		return true;
	}

	bool MemMapFile::Skip(size_t offset)
	{
		if (m_pCur + offset > m_pData + m_szFile) return false;
		m_pCur = m_pCur + offset;
		return true;
	}

	bool MemMapFile::IsEOF()
	{
		return (m_pCur >= m_pData + m_szFile);
	}

	void MemMapFile::Flush()
	{}

	void MemMapFile::Close()
	{
#if K3DPLATFORM_OS_WIN
		UnmapViewOfFile(m_pData);
		CloseHandle(m_FileMappingHandle);
		//CloseHandle( m_FileHandle );
#else
		munmap(m_pData, m_szFile);
		close(m_Fd);
#endif
	}

	MemMapFile* MemMapFile::CreateIOInterface()
	{
		return new MemMapFile;
	}
}