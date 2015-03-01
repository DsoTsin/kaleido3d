#include "Kaleido3D.h"
#include "IPv4Address.h"

#include <regex>

namespace k3d
{
	static const std::string ip_reg = "(25[0-4]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[1-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[.](25[0-4]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[1-9])";
	static const std::string ip_with_port_reg = "(25[0-4]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[1-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[.](25[0-4]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[1-9])[:][[:digit:]]";

	IPv4Address::IPv4Address(const char * ip)
	{
		::memset(&m_Addr, 0, sizeof(m_Addr));
		m_Addr.sin_family = AF_INET;
		std::regex pattern1(ip_reg);
		std::regex pattern2(ip_with_port_reg);
		std::smatch match;
		const auto & ipStr = std::string(ip);
		if (std::regex_match(ipStr, match, pattern1))
		{
#if K3DPLATFORM_OS_WIN
			m_Addr.sin_addr.S_un.S_addr = ::inet_addr(ip);
#else
			::inet_pton(AF_INET, ip, &m_Addr.sin_addr);
#endif
		}
		else if (std::regex_match(ipStr, match, pattern2))
		{
			char ipbuff[256] = { 0 };
			int port = 0;
			std::sscanf(ip, "%s:%d", ipbuff, &port);

			m_Addr.sin_port = htons((u_short)port);
#if K3DPLATFORM_OS_WIN
			m_Addr.sin_addr.S_un.S_addr = ::inet_addr(ipbuff);
#else
			::inet_pton(AF_INET, ipbuff, &m_Addr.sin_addr);
#endif
		}
		else
		{
			//error
		}
	}
	void IPv4Address::SetIpPort(uint32 port)
	{
		m_Addr.sin_port = htons((u_short)port);
	}
    IPv4Address * IPv4Address::Clone() const
    {
        return new IPv4Address(*this);
    }
}