#include "Kaleido3D.h"

#include "Socket.h"
#include "IPv4Address.h"

namespace k3d 
{
	Socket::Socket(SockType const & type)
#if K3DPLATFORM_OS_WIN
		: m_SockFd(INVALID_SOCKET)
#else
		: m_SockFd(-1)
#endif
		, m_SockType(type)
		, m_IsBlocking(true)
	{
	}

	Socket::~Socket()
	{
	}

	bool Socket::IsValid()
	{
#if K3DPLATFORM_OS_WIN
		return m_SockFd != INVALID_SOCKET;
#else
		return m_SockFd != -1;
#endif
	}

	void Socket::SetTimeOutOpt(SoToOpt opt, uint32 milleseconds)
	{
		if (!IsValid())
			return;
		struct timeval timeo;
		timeo.tv_sec = milleseconds / 1000;
		timeo.tv_usec = (milleseconds % 1000) * 1000;
		int opts[] = {
			SO_RCVTIMEO,
			SO_SNDTIMEO,
		};
		int ret = ::setsockopt(m_SockFd, SOL_SOCKET, opts[(uint32)opt], (const char*)&timeo, sizeof(timeo));
		if (ret == -1)
		{

		}
	}

	void Socket::SetBlocking(bool block)
	{
		if (!IsValid()) {
			return;
		}
#if K3DPLATFORM_OS_WIN
		unsigned long ul = block ? 0 : 1;
		ioctlsocket(m_SockFd, FIONBIO, &ul);
#else
		int flag = fcntl(handle, F_GETFL, 0);
		flag = block ? (flag & ~O_NONBLOCK) : (flag | O_NONBLOCK);
		fcntl(handle, F_SETFL, flag);
#endif
	}

	void Socket::Connect(IPv4Address const & ipAddr)
	{
		if (!IsValid())
			return;
		int rm = ::connect(m_SockFd, (sockaddr*)&ipAddr.m_Addr, sizeof(ipAddr.m_Addr));
		assert(rm >= 0);
	}

	void Socket::Listen(int maxConn)
	{
		::listen(m_SockFd, maxConn);
	}

	void Socket::Bind(IPv4Address const & ipAddr)
	{
		::bind(m_SockFd, (sockaddr*)&ipAddr.m_Addr, sizeof(ipAddr.m_Addr));
	}

	SocketHandle Socket::Accept(IPv4Address & ipAddr)
	{
		int  len = 0;
		return ::accept(m_SockFd, (sockaddr*)&ipAddr.m_Addr, &len);
	}

	void Socket::Create()
	{
		if (IsValid())
			return;
		m_SockFd = ::socket(AF_INET, m_SockType == SockType::TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
	}

	void Socket::Close()
	{
		if (IsValid())
		{
#if K3DPLATFORM_OS_WIN
			::closesocket(m_SockFd);
			m_SockFd = INVALID_SOCKET;
#else
			close(handle);
			handle = -1;
#endif
		}
	}
	
//========================================================
	namespace RAII {
#if K3DPLATFORM_OS_WIN
		struct SocketInitializer
		{
			SocketInitializer()
			{
				WSADATA init;
				::WSAStartup(MAKEWORD(2, 2), &init);
			}

			~SocketInitializer()
			{
				::WSACleanup();
			}
		};

		SocketInitializer globalInitializer;
#endif
	}
//========================================================
}