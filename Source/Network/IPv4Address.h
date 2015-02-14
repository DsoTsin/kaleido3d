#pragma once
#include "Socket.h"

namespace k3d
{
	class IPv4Address 
	{
	public:
		explicit IPv4Address(const char* ip);
		void SetIpPort(uint32 port);

	private:

		friend class Socket;
		sockaddr_in m_Addr;
	};
}