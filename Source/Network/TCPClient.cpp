//
//  TCPClient.cpp
//  Kaleido3D
//
//  Created by QinZhou on 15/3/1.
//
//

#include "Kaleido3D.h"
#include "TCPClient.h"

namespace k3d {
    
    TCPClient::TCPClient()
    : Socket(SockType::TCP)
    {
        Create();
    }
    
    Ref<INetConn> TCPClient::ConnectTo(IPv4Address const & ip)
    {
        Connect(ip);
        return Ref<INetConn>(new NetConnection(GetHandle(), ip));
    }
}