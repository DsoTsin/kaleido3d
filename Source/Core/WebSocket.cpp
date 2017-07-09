#include "Kaleido3D.h"
#include "WebSocket.h"
#include "Utils/Base64.h"
#include "Utils/SHA1.h"

using namespace Os;
using namespace std;

namespace net
{
	const size_t BUF_SIZE = 4096;

    WebSocket::WebSocket() : Socket(SockType::TCP)
    {
    }

    WebSocket::~WebSocket()
    {
    }

	Os::SocketHandle WebSocket::Accept(Os::IPv4Address & ipAddr)
	{
		Os::SocketHandle handle = Socket::Accept(ipAddr);
		unsigned char buffer[BUF_SIZE] = {0};
		uint64 recvLen = Socket::Receive(handle, buffer, BUF_SIZE);
		if (recvLen < BUF_SIZE)
		{
			if (ParseHandshake(buffer, recvLen) == OPENING_FRAME)
			{
				string answer = AnswerHandshake();
				uint64 sent = Socket::Send(handle, answer);
				if (sent == answer.size())
				{
					return handle;
				}
			}
		}
		return 0;
	}

	uint64 WebSocket::Receive(Os::SocketHandle remote, void * pData, uint32 recvLen)
	{
		unsigned char buffer[BUF_SIZE] = { 0 };
		uint64 realRecvLen = Socket::Receive(remote, buffer, BUF_SIZE);
		if (realRecvLen < BUF_SIZE) // received all data
		{
			int out_length = 0;
			m_CurrentFameType = GetFrame(buffer, realRecvLen, (unsigned char*)pData, recvLen, &out_length);
			if (m_CurrentFameType != ERROR_FRAME)
			{
				return out_length;
			}
			else
			{
				return (uint64)-1;
			}
		}
		return 0;
	}

	uint64 WebSocket::Send(Os::SocketHandle remote, const char * pData, uint32 sendLen)
	{
		unsigned char buffer[BUF_SIZE] = { 0 };
		int realSize = MakeFrame(m_CurrentFameType, pData, sendLen, buffer, BUF_SIZE);
		if (realSize < BUF_SIZE)
		{
			uint64 sent = Socket::Send(remote, (const char*)buffer, realSize);
			if (sent == realSize)
			{
				return sent;
			}
		}
		return 0;
	}

    WebSocketFrameType WebSocket::ParseHandshake(unsigned char *input_frame, size_t input_len)
    {
        // 1. copy char*/len into string
        // 2. try to parse headers until \r\n occurs
        string headers((char*)input_frame, input_len);
		size_t header_end = headers.find("\r\n\r\n");

        if(header_end == string::npos) { // end-of-headers not found - do not parse
            return INCOMPLETE_FRAME;
        }

        headers.resize(header_end); // trim off any data we don't need after the headers
        vector<string> headers_rows = Explode(headers, string("\r\n"));
        for(int i=0; i<headers_rows.size(); i++) {
            string& header = headers_rows[i];
            if(header.find("GET") == 0) {
                vector<string> get_tokens = Explode(header, string(" "));
                if(get_tokens.size() >= 2) {
                    this->resource = get_tokens[1];
                }
            }
            else {
				size_t pos = header.find(":");
                if(pos != string::npos) {
                    string header_key(header, 0, pos);
                    string header_value(header, pos+1);
                    header_value = Trim(header_value);
                    if(header_key == "Host") this->host = header_value;
                    else if(header_key == "Origin") this->origin = header_value;
                    else if(header_key == "Sec-WebSocket-Key") this->key = header_value;
                    else if(header_key == "Sec-WebSocket-Protocol") this->protocol = header_value;
                }
            }
        }
		return OPENING_FRAME;
    }

    string WebSocket::Trim(string str)
    {
        auto whitespace = " \t\r\n";
        string::size_type pos = str.find_last_not_of(whitespace);
        if(pos != string::npos) {
            str.erase(pos + 1);
            pos = str.find_first_not_of(whitespace);
            if(pos != string::npos) str.erase(0, pos);
        }
        else {
            return string();
        }
        return str;
    }

    vector<string> WebSocket::Explode(
            string  theString,
            string  theDelimiter,
            bool    theIncludeEmptyStrings)
    {
        //printf("EXPLODE\n");
        //UASSERT( theDelimiter.size(), >, 0 );

        vector<string> theStringVector;
		size_t  start = 0, end = 0, length = 0;

        while ( end != string::npos )
        {
            end = theString.find( theDelimiter, start );

            // If at end, use length=maxLength.  Else use length=end-start.
            length = (end == string::npos) ? string::npos : end - start;

            if (theIncludeEmptyStrings
                || (   ( length > 0 ) /* At end, end == length == string::npos */
                       && ( start  < theString.size() ) ) )
                theStringVector.push_back( theString.substr( start, length ) );

            // If at end, use start=maxSize.  Else use start=end+delimiter.
            start = (   ( end > (string::npos - theDelimiter.size()) )
                        ?  string::npos  :  end + theDelimiter.size()     );
        }
        return theStringVector;
    }

    string WebSocket::AnswerHandshake()
    {
        unsigned char digest[20]; // 160 bit sha1 digest

        string answer;
        answer += "HTTP/1.1 101 Switching Protocols\r\n";
        answer += "Upgrade: WebSocket\r\n";
        answer += "Connection: Upgrade\r\n";
        if(this->key.length() > 0) {
            string accept_key;
            accept_key += this->key;
            accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //RFC6544_MAGIC_KEY

            //printf("INTERMEDIATE_KEY:(%s)\n", accept_key.data());

            SHA1 sha;
            sha.Input(accept_key.data(), (unsigned int)accept_key.size());
            sha.Result((unsigned*)digest);

            //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

            //little endian to big endian
            for(int i=0; i<20; i+=4) {
                unsigned char c;

                c = digest[i];
                digest[i] = digest[i+3];
                digest[i+3] = c;

                c = digest[i+1];
                digest[i+1] = digest[i+2];
                digest[i+2] = c;
            }

            //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

            accept_key = Base64::Encode((const unsigned char *)digest, 20); //160bit = 20 bytes/chars

            answer += "Sec-WebSocket-Accept: "+(accept_key)+"\r\n";
        }
        if(this->protocol.length() > 0) {
            answer += "Sec-WebSocket-Protocol: "+(this->protocol)+"\r\n";
        }
        answer += "\r\n";
        return answer;

        //return WS_OPENING_FRAME;
    }

    int WebSocket::MakeFrame(WebSocketFrameType frame_type, const char* msg, int msg_length, unsigned char* buffer, int buffer_size)
    {
        int pos = 0;
        int size = msg_length;
        buffer[pos++] = (unsigned char)frame_type; // text frame

        if(size <= 125) {
            buffer[pos++] = (unsigned char)size;
        }
        else if(size <= 65535) {
            buffer[pos++] = 126; //16 bit length follows

            buffer[pos++] = (size >> 8) & 0xFF; // leftmost first
            buffer[pos++] = size & 0xFF;
        }
        else { // >2^16-1 (65535)
            buffer[pos++] = 127; //64 bit length follows

            // write 8 bytes length (significant first)

            // since msg_length is int it can be no longer than 4 bytes = 2^32-1
            // padd zeroes for the first 4 bytes
            for(int i=3; i>=0; i--) {
                buffer[pos++] = 0;
            }
            // write the actual 32bit msg_length in the next 4 bytes
            for(int i=3; i>=0; i--) {
                buffer[pos++] = ((size >> 8*i) & 0xFF);
            }
        }
        memcpy((void*)(buffer+pos), msg, size);
        return (size+pos);
    }

    WebSocketFrameType WebSocket::GetFrame(unsigned char* in_buffer, size_t in_length, unsigned char* out_buffer, int out_size, int* out_length)
    {
        if(in_length < 3) return INCOMPLETE_FRAME;

        unsigned char msg_opcode = in_buffer[0] & 0x0F;
        unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
        unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

        // *** message decoding

        int payload_length = 0;
        int pos = 2;
        int length_field = in_buffer[1] & (~0x80);
        unsigned int mask = 0;

        //printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

        if(length_field <= 125) {
            payload_length = length_field;
        }
        else if(length_field == 126) { //msglen is 16bit!
            payload_length = in_buffer[2] + (in_buffer[3]<<8);
            pos += 2;
        }
        else if(length_field == 127) { //msglen is 64bit!
            payload_length = in_buffer[2] + (in_buffer[3]<<8);
            pos += 8;
        }
        //printf("PAYLOAD_LEN: %08x\n", payload_length);
        if(in_length < payload_length+pos) {
            return INCOMPLETE_FRAME;
        }

        if(msg_masked) {
            mask = *((unsigned int*)(in_buffer+pos));
            //printf("MASK: %08x\n", mask);
            pos += 4;

            // unmask data:
            unsigned char* c = in_buffer+pos;
            for(int i=0; i<payload_length; i++) {
                c[i] = c[i] ^ ((unsigned char*)(&mask))[i%4];
            }
        }

        if(payload_length > out_size) {
            //TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
        }

        memcpy((void*)out_buffer, (void*)(in_buffer+pos), payload_length);
        out_buffer[payload_length] = 0;
        *out_length = payload_length+1;

        //printf("TEXT: %s\n", out_buffer);

        if(msg_opcode == 0x0) return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME; // continuation frame ?
        if(msg_opcode == 0x1) return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME;
        if(msg_opcode == 0x2) return (msg_fin)?BINARY_FRAME:INCOMPLETE_BINARY_FRAME;
        if(msg_opcode == 0x9) return PING_FRAME;
        if(msg_opcode == 0xA) return PONG_FRAME;

        return ERROR_FRAME;
    }
}


