#ifndef __WebSocket_H__
#define __WebSocket_H__

#include "Os.h"

#include <vector>

namespace net
{
    enum WebSocketFrameType {
        ERROR_FRAME=0xFF00,
        INCOMPLETE_FRAME=0xFE00,

        OPENING_FRAME=0x3300,
        CLOSING_FRAME=0x3400,

        INCOMPLETE_TEXT_FRAME=0x01,
        INCOMPLETE_BINARY_FRAME=0x02,

        TEXT_FRAME=0x81,
        BINARY_FRAME=0x82,

        PING_FRAME=0x19,
        PONG_FRAME=0x1A
    };

    class K3D_API WebSocket : public Os::Socket
    {
    public:
							WebSocket();
        virtual				~WebSocket();

		Os::SocketHandle	Accept(Os::IPv4Address & ipAddr) override;
		uint64				Receive(Os::SocketHandle reomte, void * pData, uint32 recvLen) override;
		uint64				Send(Os::SocketHandle remote, const char * pData, uint32 sendLen) override;

    protected:

		WebSocketFrameType	ParseHandshake(unsigned char *input_frame, size_t input_len);
		std::string			AnswerHandshake();
		WebSocketFrameType	GetFrame(unsigned char* in_buffer, size_t in_length, unsigned char* out_buffer, int out_size, int* out_length);
		int					MakeFrame(WebSocketFrameType frame_type, const char* msg, int msg_len, unsigned char* buffer, int buffer_len);
		std::string			Trim(std::string str);
		std::vector<std::string> Explode(
			std::string theString, std::string theDelimiter,
			bool theIncludeEmptyStrings = false);

		WebSocketFrameType	m_CurrentFameType;
        std::string resource;
        std::string host;
        std::string origin;
        std::string protocol;
        std::string key;
    };
}

#endif