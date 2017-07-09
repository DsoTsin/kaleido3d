#include "Common.h"
#include <sstream>
#include <KTL/String.hpp>

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace net;

class TestServer : public WebSocket
{
public:
	static const uint32 BUF_LEN = 8192;

	TestServer() : WebSocket() {  }

	void BindAndListen()
	{
		Create();
		Os::IPv4Address addr("192.168.0.112:7000");
		this->SetBlocking(true);
		Bind(addr);
		Listen(10);
	}
	
	void Start()
	{
		Os::IPv4Address unnamedClient("");
		Os::SocketHandle client;

		while (true)
		{
			client = this->Accept(unnamedClient);
			if (client == -1)
			{
                Os::Sleep(1000);
				continue;
			}

			while (true)
			{
				char buffer[BUF_LEN + 1] = { 0 };
				uint64 recvLen = this->Receive(client, buffer, BUF_LEN);
				if (recvLen > 0 && recvLen < BUF_LEN)
				{
					//OutputDebugStringA(buffer);
				}
				else
				{
					if (recvLen <= 0) break;
					continue;
				}
			}
		}
	}

	std::stringstream m_LogStream;
};

void TestWebSocket()
{
	TestServer server;
	server.BindAndListen();
		server.Start();
}

int main(int argc, char**argv)
{
	TestWebSocket();
	return 0;
}

