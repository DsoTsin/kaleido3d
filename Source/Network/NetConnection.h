//
//  NetConnection.h
//  Kaleido3D
//
//  Created by QinZhou on 15/3/1.
//
//
#pragma once
#ifndef __Kaleido3D__NetConnection__
#define __Kaleido3D__NetConnection__

#include "Socket.h"
#include "IPv4Address.h"

namespace k3d {
    
    struct INetConn
    {
        virtual ~INetConn(){}
        virtual void AddRef() = 0;
        virtual void Release() = 0;
        virtual uint32 Send(std::string const & buffer) = 0;
        virtual std::string Receive(uint32 size) = 0;
    };
    
    class NetConnection : public INetConn
    {
    public:
        
        NetConnection(SocketHandle const & sock, IPv4Address const & ip);
        ~NetConnection();
        
        void AddRef() override;
        void Release() override;
        
        uint32 Send(std::string const & buffer) override;
        std::string Receive(uint32 len) override;
        
    private:
        uint32          m_RefCnt;
        SocketHandle    m_SideSockFd;
        IPv4Address*    m_SideIP;
    };
}

#endif /* defined(__Kaleido3D__NetConnection__) */
