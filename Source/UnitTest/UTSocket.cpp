#include "Kaleido3D.h"
#include <Network/Socket.h>
#include <Network/IPv4Address.h>

#pragma comment(lib, "Ws2_32.lib")
using namespace k3d;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	Socket socket(SockType::TCP);
	
	return 0;
}
