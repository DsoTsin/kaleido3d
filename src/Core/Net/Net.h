#ifndef __WebSocket_H__
#define __WebSocket_H__

namespace k3d
{
    namespace net
    {
        class WebSocketImpl;

        class K3D_CORE_API WebSocket : public os::Socket
        {
            friend class WebSocketImpl;
        public:
            WebSocket();
            virtual				~WebSocket();

            os::Socket*	        Accept(os::IpAddress const& Ip) override;
            I32				    Receive(void * pData, I32 recvLen) override;
            I32				    Send(const char * pData, I32 sendLen) override;

        protected:
            WebSocketImpl*      d;
        };

        class K3D_CORE_API HttpRequest
        {
        public:
            HttpRequest();
        };

        class K3D_CORE_API HttpClient
        {
        public:
            HttpClient();
            ~HttpClient();


        };


    }
}

#endif