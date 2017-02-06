#ifndef __Os_h__
#define __Os_h__

#include <Interface/IIODevice.h>
#include <Config/OSHeaders.h>

#include <functional>
#include <map>

/**
 * This module provides facilities on OS like:
 * File,Directory,Socket,Threading
 */
namespace Os
{
	class K3D_API File : public ::IIODevice
	{
	public:
		File();
		explicit File(const char *fileName);

		~File();

		bool      Open(IOFlag flag);
		bool      Open(const char* fileName, IOFlag flag);
#if K3DPLATFORM_OS_WIN
		bool	  Open(const WCHAR *fileName, IOFlag flag);
#endif
		int64     GetSize();

		bool      IsEOF();

		size_t    Read(char *ptr, size_t len);
		size_t    Write(const void *ptr, size_t len);

		bool      Seek(size_t offset);
		bool      Skip(size_t offset);

		void      Flush();
		void      Close();

		uint64    LastModified() const;

		static File * CreateIOInterface();

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

	class K3D_API MemMapFile : public ::IIODevice
	{
	public:
		MemMapFile();
		~MemMapFile();

		int64     GetSize();
		//---------------------------------------------------------
		bool      Open(const char* fileName, IOFlag mode);
#if K3DPLATFORM_OS_WIN
		bool	  Open(const WCHAR *fileName, IOFlag flag);
#endif
		size_t    Read(char * data_ptr, size_t len);
		size_t    Write(const void *, size_t);
		bool      Seek(size_t offset);
		bool      Skip(size_t offset);
		bool      IsEOF();
		void      Flush();
		void      Close();
		//---------------------------------------------------------

		//---------------------------------------------------------
		/// FileData
		/// \brief FileData
		/// \return data const pointer
		kByte* FileData() { return m_pData; }

		template <class T>
		/// Convert FileBlocks To Class
		/// \brief ConvertToClass
		/// \param address_of_file
		/// \param object_offset
		/// \return Object Pointer
		static T*   ConvertToClass(kByte * &address_of_file, uint32 object_offset)
		{
			kByte *objectptr = address_of_file + object_offset;
			address_of_file += sizeof(T) + object_offset;
			return reinterpret_cast<T*>(objectptr);
		}
		//---------------------------------------------------------

		/// General Interface For GetIODevice
		/// \brief CreateIOInterface
		/// \return An IIODevice Pointer
		static MemMapFile* CreateIOInterface();
	private:

#ifdef K3DPLATFORM_OS_WIN
		HANDLE    m_FileHandle;
		HANDLE    m_FileMappingHandle;
#else
		int       m_Fd;
#endif
		size_t    m_szFile;
		kByte *   m_pData;
		kByte *   m_pCur;
	};
    
    extern K3D_API int Exec(const ::k3d::kchar * cmd, ::k3d::kchar *const *argv);
	extern K3D_API bool MakeDir(const ::k3d::kchar * name);
	extern K3D_API bool Exists(const ::k3d::kchar * name);
	extern K3D_API void Sleep(uint32 ms);
	extern K3D_API bool Copy(const ::k3d::kchar * src, const ::k3d::kchar * target);
	extern K3D_API bool Remove(const ::k3d::kchar * name);
	typedef void(*PFN_FileProcessRoutine)(const ::k3d::kchar * path, bool isDir);
	extern K3D_API bool ListFiles(const ::k3d::kchar * srcPath, PFN_FileProcessRoutine);
	extern K3D_API uint32 GetCpuCoreNum();
	extern K3D_API float* GetCpuUsage();

	enum class ThreadPriority 
	{
		Low,
		Normal,
		High,
		RealTime
	};

	enum class ThreadStatus 
	{
		Ready,
		Running,
		Finish
	};
	
	struct MutexPrivate;
	class K3D_API Mutex
	{
	public:
		Mutex();
		~Mutex();

		void Lock();
		void UnLock();
		friend class ConditionVariable;

		struct AutoLock
		{
			AutoLock()
			{
				m_Mutex = new Mutex;
				m_Mutex->Lock();
			}

			explicit AutoLock(Mutex * mutex, bool lostOwnerShip = false)
				: m_OnwerShipGot(lostOwnerShip)
				, m_Mutex(mutex)
			{
			}

			~AutoLock()
			{
				m_Mutex->UnLock();
				if (m_OnwerShipGot)
				{
					delete m_Mutex;
				}
			}
		private:
			bool	m_OnwerShipGot;
			Mutex	*m_Mutex;
		};

	private:
		MutexPrivate * m_Impl;
	};


	struct ConditionVariablePrivate;
	class K3D_API ConditionVariable {
	public:
		ConditionVariable();
		~ConditionVariable();

		void Wait(Mutex * mutex);
		void Wait(Mutex * mutex, uint32 milliseconds);
		void Notify();
		void NotifyAll();

		ConditionVariable(const ConditionVariable &) = delete;
		ConditionVariable(const ConditionVariable &&) = delete;

	protected:

		ConditionVariablePrivate * m_Impl;
	};

	class K3D_API Thread {
	public:
		// static functions
		static void SleepForMilliSeconds(uint32_t millisecond);
		//static void Yield();
		static uint32_t GetId();

	public:
		typedef void * Handle;
		typedef std::function<void()> Call;

		Thread();

		explicit		Thread(std::string const & name, ThreadPriority priority = ThreadPriority::Normal);
		explicit		Thread(Call && callback, std::string const & name, ThreadPriority priority = ThreadPriority::Normal);

		virtual			~Thread();

		void			SetPriority(ThreadPriority prio);
		void			Start();
		void			Join();
		void			Terminate();

		ThreadStatus	GetThreadStatus();
		std::string		GetName();

	public:
		static std::string  GetCurrentThreadName();
		static void			SetCurrentThreadName(std::string const& name);

	private:
		Call				m_ThreadCallBack;
		std::string         m_ThreadName;
		ThreadPriority	    m_ThreadPriority;
		uint32_t			m_StackSize;
		ThreadStatus		m_ThreadStatus;
		Handle				m_ThreadHandle;

	private:

		static void* STD_CALL Run(void*);
		static std::map<uint32, Thread*>	s_ThreadMap;

	};

	class SockImpl;

	class K3D_API IPv4Address
	{
	public:
		explicit IPv4Address(const char* ip);
		void SetIpPort(uint32 port);

		IPv4Address * Clone() const;
	private:

		friend class Socket;
		sockaddr_in m_Addr;
	};

#if K3DPLATFORM_OS_WIN
	typedef UINT_PTR	SocketHandle;
#else
	typedef int			SocketHandle;
#endif

	enum class SockStatus
	{
		Connected,
		DisConnected,
		Error
	};

	enum class SockType
	{
		TCP,
		UDP,
		RAW
	};

	enum class SoToOpt : uint32
	{
		Receive = 0,
		Send = 1,
		/*Connect = 2*/
	};

	class K3D_API Socket
	{
	public:
		explicit Socket(SockType const & type);

		virtual ~Socket();

		bool IsValid();

		void SetTimeOutOpt(SoToOpt opt, uint32 milleseconds);
		void SetBlocking(bool block);

	protected:

		void Create();
		void Bind(IPv4Address const & ipAddr);
		void Listen(int maxConn);
		virtual void Connect(IPv4Address const & ipAddr);
		virtual void Close();
		virtual SocketHandle Accept(IPv4Address & ipAddr);
		virtual uint64 Receive(SocketHandle reomte, void * pData, uint32 recvLen);
		virtual uint64 Send(SocketHandle remote, const char * pData, uint32 sendLen);
		virtual uint64 Send(SocketHandle remote, std::string const & buffer);
		SocketHandle GetHandle() { return m_SockFd; }

	private:

		SocketHandle	m_SockFd;
		SockType		m_SockType;
#if K3DPLATFORM_OS_WIN
		bool			m_IsBlocking;
#endif
	};
}

KTYPE_ISTYPE_TEMPLATE(Os::File, IIODevice);
KTYPE_ISTYPE_TEMPLATE(Os::MemMapFile, IIODevice);

#endif
