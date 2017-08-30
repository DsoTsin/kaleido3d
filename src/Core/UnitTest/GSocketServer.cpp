#include <CoreMinimal.h>

#if K3DPLATFORM_OS_WINDOWS
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d::os;

int main()
{
    Socket server(SockType::TCP);
    server.SetReuseAddr(true);
    server.SetBlocking(false);
    server.Bind(IpAddress("127.0.0.1:8999"));
    server.Listen(100);

    SocketSet Rs;
    Rs.Add(&server);
    SocketSet Ws;
    while (true)
    {
        int nRet = SocketSet::Select(Rs, Ws, 1000);
        if (nRet > 0)
        {
            for (auto R : Rs)
            {
                IpAddress Ip(nullptr);
                if (R == &server)
                {
                    Socket* C = R->Accept(Ip);
                    if (C)
                    {
                        Rs.Add(C);
                    }
                }
                else
                {
                    char Buffer[1024] = { 0 };
                    int Recv = R->Receive(Buffer, 1024);
                    if (Recv>0)
                        printf("kkkkk: R %s\n", Buffer);
                }
            }
        }
    }
}