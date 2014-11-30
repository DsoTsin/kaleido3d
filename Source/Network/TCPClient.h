//
//  TCPClient.h
//  Kaleido3D
//
//  Created by QinZhou on 15/3/1.
//
//

#ifndef __Kaleido3D__TCPClient__
#define __Kaleido3D__TCPClient__

#include <KTL/RefCount.hpp>

#include "Socket.h"
#include "IPv4Address.h"
#include "NetConnection.h"

namespace k3d {
    
    class TCPClient : public Socket
    {
    public:
        TCPClient();
        
        Ref<INetConn> ConnectTo(IPv4Address const & ip);
        
    };
    
}

#endif /* defined(__Kaleido3D__TCPClient__) */
