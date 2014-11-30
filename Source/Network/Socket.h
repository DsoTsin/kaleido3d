#pragma once
#include <Config/OSHeaders.h>
#if K3DPLATFORM_OS_WIN
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace k3d
{
	class SockImpl;
	class IPv4Address;

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

	class Socket 
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
		void Connect(IPv4Address const & ipAddr);
		void Close();
		SocketHandle Accept(IPv4Address & ipAddr);

        SocketHandle GetHandle() { return m_SockFd; }
        
	private:

		SocketHandle	m_SockFd;
		SockType		m_SockType;
		bool			m_IsBlocking;
	};
}