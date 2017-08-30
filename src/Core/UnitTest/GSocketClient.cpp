#include <CoreMinimal.h>
#if K3DPLATFORM_OS_WINDOWS
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d::os;
int main()
{
    Socket Sc(SockType::TCP);
    Sc.Connect(IpAddress("127.0.0.1:8999"));
    Sc.Send(k3d::String("77777"));
}