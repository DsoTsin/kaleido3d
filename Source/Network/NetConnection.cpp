//
//  NetConnection.cpp
//  Kaleido3D
//
//  Created by QinZhou on 15/3/1.
//
//

#include "Kaleido3d.h"
#include "NetConnection.h"

namespace k3d {
    
    NetConnection::NetConnection(SocketHandle const & sock, IPv4Address const & ip)
    {
        m_RefCnt = 1;
        m_SideSockFd = sock;
        m_SideIP = ip.Clone();
    }
    
    NetConnection::~NetConnection()
    {
    }
    
    void NetConnection::AddRef()
    {
        m_RefCnt++;
    }
    
    void NetConnection::Release()
    {
        m_RefCnt--;
        if(m_RefCnt==0)
        {
#if K3DPLATFORM_OS_WIN
            ::closesocket(m_SideSockFd);
#else
            close(m_SideSockFd);
            m_SideSockFd = -1;
#endif
        }
    }
    
    std::string NetConnection::Receive(uint32 len)
    {
        char * bytes = new char[len+1];
        ::recv(m_SideSockFd, bytes, len, 0);
        return std::string((const char*)bytes);
    }
    
    uint32 NetConnection::Send(std::string const & buffer)
    {
        ::send(m_SideSockFd, buffer.c_str(), buffer.size(), 0);
        return 0;
    }
}
