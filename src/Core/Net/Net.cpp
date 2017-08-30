#include "CoreMinimal.h"
#include "Base/Encoder.h"

namespace k3d
{
    namespace net
    {
        enum WebSocketFrameType {
            ERROR_FRAME = 0xFF00,
            INCOMPLETE_FRAME = 0xFE00,

            OPENING_FRAME = 0x3300,
            CLOSING_FRAME = 0x3400,

            INCOMPLETE_TEXT_FRAME = 0x01,
            INCOMPLETE_BINARY_FRAME = 0x02,

            TEXT_FRAME = 0x81,
            BINARY_FRAME = 0x82,

            PING_FRAME = 0x19,
            PONG_FRAME = 0x1A
        };

        using k3d::String;

        const size_t BUF_SIZE = 4096;

        class WebSocketImpl
        {
        public:
            WebSocket* Owner;
            WebSocketImpl(WebSocket* In) : Owner(In) {}


            WebSocketFrameType	ParseHandshake(unsigned char *input_frame, size_t input_len);
            k3d::String			AnswerHandshake();
            WebSocketFrameType	GetFrame(unsigned char* in_buffer, size_t in_length, unsigned char* out_buffer, int out_size, int* out_length);
            int					MakeFrame(WebSocketFrameType frame_type, const char* msg, int msg_len, unsigned char* buffer, int buffer_len);
            k3d::String			Trim(k3d::String str);
            k3d::DynArray<k3d::String> Explode(
                k3d::String theString, k3d::String theDelimiter,
                bool theIncludeEmptyStrings = false);

            WebSocketFrameType	m_CurrentFameType;
            k3d::String resource;
            k3d::String host;
            k3d::String origin;
            k3d::String protocol;
            k3d::String key;

        };


        WebSocket::WebSocket() : Socket(os::SockType::TCP), d(nullptr)
        {
            d = new WebSocketImpl(this);
        }

        WebSocket::~WebSocket()
        {
            if (d)
            {
                delete d;
                d = nullptr;
            }
        }

        os::Socket* WebSocket::Accept(os::IpAddress const& ipAddr)
        {
            os::Socket* NewSock = Socket::Accept(ipAddr);
            unsigned char buffer[BUF_SIZE] = { 0 };
            U64 recvLen = NewSock->Receive(buffer, BUF_SIZE);
            if (recvLen < BUF_SIZE)
            {
                if (d->ParseHandshake(buffer, recvLen) == OPENING_FRAME)
                {
                    k3d::String answer = d->AnswerHandshake();
                    I32 sent = NewSock->Send(answer);
                    if (sent == answer.Length())
                    {
                        return NewSock;
                    }
                }
            }
            return NewSock;
        }

        I32 WebSocket::Receive(void * pData, I32 recvLen)
        {
            unsigned char buffer[BUF_SIZE] = { 0 };
            I32 realRecvLen = Receive(buffer, BUF_SIZE);
            if (realRecvLen < BUF_SIZE) // received all data
            {
                int out_length = 0;
                d->m_CurrentFameType = d->GetFrame(buffer, realRecvLen, (unsigned char*)pData, (int)recvLen, &out_length);
                if (d->m_CurrentFameType != ERROR_FRAME)
                {
                    return out_length;
                }
                else
                {
                    return -1;
                }
            }
            return 0;
        }

        I32 WebSocket::Send(const char * pData, I32 sendLen)
        {
            unsigned char buffer[BUF_SIZE] = { 0 };
            int realSize = d->MakeFrame(d->m_CurrentFameType, pData, (int)sendLen, buffer, BUF_SIZE);
            if (realSize < BUF_SIZE)
            {
                I32 sent = Send((const char*)buffer, realSize);
                if (sent == realSize)
                {
                    return sent;
                }
            }
            return 0;
        }

        WebSocketFrameType WebSocketImpl::ParseHandshake(unsigned char *input_frame, size_t input_len)
        {
            // 1. copy char*/len into string
            // 2. try to parse headers until \r\n occurs
            String headers((char*)input_frame, input_len);
            auto header_end = headers.Find("\r\n\r\n");

            if (header_end == String::npos) { // end-of-headers not found - do not parse
                return INCOMPLETE_FRAME;
            }

            headers.Resize(header_end); // trim off any data we don't need after the headers
            k3d::DynArray<String> headers_rows = Explode(headers, String("\r\n"));
            for (int i = 0; i < headers_rows.Count(); i++) {
                String& header = headers_rows[i];
                if (header.Find("GET") == 0) {
                    k3d::DynArray<String> get_tokens = Explode(header, String(" "));
                    if (get_tokens.Count() >= 2) {
                        this->resource = get_tokens[1];
                    }
                }
                else {
                    auto pos = header.Find(":");
                    if (pos != String::npos) {
                        //String header_key(header, 0, pos);
                        //String header_value(header, pos+1);
                        //header_value = Trim(header_value);
                        //if(header_key == "Host") this->host = header_value;
                        //else if(header_key == "Origin") this->origin = header_value;
                        //else if(header_key == "Sec-WebSocket-Key") this->key = header_value;
                        //else if(header_key == "Sec-WebSocket-Protocol") this->protocol = header_value;
                    }
                }
            }
            return OPENING_FRAME;
        }

        k3d::String WebSocketImpl::Trim(k3d::String str)
        {
            String whitespace = " \t\r\n";
            auto pos = str.FindLastNotOf(*whitespace);
            if (pos != String::npos) {
                str.Erase(pos + 1);
                pos = str.FindFirstNotOf(*whitespace);
                if (pos != String::npos)
                {
                    str.Erase(0, pos);
                }
            }
            else {
                return String();
            }
            return String();
        }

        k3d::DynArray<String> WebSocketImpl::Explode(
            String  theString,
            String  theDelimiter,
            bool    theIncludeEmptyStrings)
        {
            k3d::DynArray<String> theStringVector;
            int start = 0, end = 0, length = 0;

            while (end != String::npos)
            {
                //end = theString.Find( theDelimiter, start );

                // If at end, use length=maxLength.  Else use length=end-start.
                length = (end == String::npos) ? String::npos : end - start;

                if (theIncludeEmptyStrings
                    || ((length > 0) /* At end, end == length == string::npos */
                        && (start < theString.Length())))
                    theStringVector.Append(theString.SubStr(start, length));

                // If at end, use start=maxSize.  Else use start=end+delimiter.
                start = ((end > (String::npos - (int)theDelimiter.Length()))
                    ? String::npos : end + (int)theDelimiter.Length());
            }
            return theStringVector;
        }

        String WebSocketImpl::AnswerHandshake()
        {
            String digest(20, '0');
            String answer;
            answer += "HTTP/1.1 101 Switching Protocols\r\n";
            answer += "Upgrade: WebSocket\r\n";
            answer += "Connection: Upgrade\r\n";
            if (this->key.Length() > 0) {
                String accept_key;
                accept_key += this->key;
                accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //RFC6544_MAGIC_KEY

                //printf("INTERMEDIATE_KEY:(%s)\n", accept_key.data());

                SHA1 sha;
                sha.Input(accept_key.CStr(), (unsigned int)accept_key.Length());
                sha.Result((unsigned*)digest.CStr());

                //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

                //little endian to big endian
                for (int i = 0; i < 20; i += 4) {
                    unsigned char c;

                    c = digest[i];
                    digest[i] = digest[i + 3];
                    digest[i + 3] = c;

                    c = digest[i + 1];
                    digest[i + 1] = digest[i + 2];
                    digest[i + 2] = c;
                }

                //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

                accept_key = k3d::Base64Encode(digest); //160bit = 20 bytes/chars

                answer += "Sec-WebSocket-Accept: " + (accept_key)+"\r\n";
            }
            if (this->protocol.Length() > 0) {
                answer += "Sec-WebSocket-Protocol: " + (this->protocol) + "\r\n";
            }
            answer += "\r\n";
            return answer;

            //return WS_OPENING_FRAME;
        }

        int WebSocketImpl::MakeFrame(WebSocketFrameType frame_type, const char* msg, int msg_length, unsigned char* buffer, int buffer_size)
        {
            int pos = 0;
            int size = msg_length;
            buffer[pos++] = (unsigned char)frame_type; // text frame

            if (size <= 125) {
                buffer[pos++] = (unsigned char)size;
            }
            else if (size <= 65535) {
                buffer[pos++] = 126; //16 bit length follows

                buffer[pos++] = (size >> 8) & 0xFF; // leftmost first
                buffer[pos++] = size & 0xFF;
            }
            else { // >2^16-1 (65535)
                buffer[pos++] = 127; //64 bit length follows

                // write 8 bytes length (significant first)

                // since msg_length is int it can be no longer than 4 bytes = 2^32-1
                // padd zeroes for the first 4 bytes
                for (int i = 3; i >= 0; i--) {
                    buffer[pos++] = 0;
                }
                // write the actual 32bit msg_length in the next 4 bytes
                for (int i = 3; i >= 0; i--) {
                    buffer[pos++] = ((size >> 8 * i) & 0xFF);
                }
            }
            memcpy((void*)(buffer + pos), msg, size);
            return (size + pos);
        }

        WebSocketFrameType WebSocketImpl::GetFrame(unsigned char* in_buffer, size_t in_length, unsigned char* out_buffer, int out_size, int* out_length)
        {
            if (in_length < 3) return INCOMPLETE_FRAME;

            unsigned char msg_opcode = in_buffer[0] & 0x0F;
            unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
            unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

            // *** message decoding

            int payload_length = 0;
            int pos = 2;
            int length_field = in_buffer[1] & (~0x80);
            unsigned int mask = 0;

            //printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

            if (length_field <= 125) {
                payload_length = length_field;
            }
            else if (length_field == 126) { //msglen is 16bit!
                payload_length = in_buffer[2] + (in_buffer[3] << 8);
                pos += 2;
            }
            else if (length_field == 127) { //msglen is 64bit!
                payload_length = in_buffer[2] + (in_buffer[3] << 8);
                pos += 8;
            }
            //printf("PAYLOAD_LEN: %08x\n", payload_length);
            if (in_length < payload_length + pos) {
                return INCOMPLETE_FRAME;
            }

            if (msg_masked) {
                mask = *((unsigned int*)(in_buffer + pos));
                //printf("MASK: %08x\n", mask);
                pos += 4;

                // unmask data:
                unsigned char* c = in_buffer + pos;
                for (int i = 0; i < payload_length; i++) {
                    c[i] = c[i] ^ ((unsigned char*)(&mask))[i % 4];
                }
            }

            if (payload_length > out_size) {
                //TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
            }

            memcpy((void*)out_buffer, (void*)(in_buffer + pos), payload_length);
            out_buffer[payload_length] = 0;
            *out_length = payload_length + 1;

            //printf("TEXT: %s\n", out_buffer);

            if (msg_opcode == 0x0) return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME; // continuation frame ?
            if (msg_opcode == 0x1) return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;
            if (msg_opcode == 0x2) return (msg_fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME;
            if (msg_opcode == 0x9) return PING_FRAME;
            if (msg_opcode == 0xA) return PONG_FRAME;

            return ERROR_FRAME;
        }
    }
}
