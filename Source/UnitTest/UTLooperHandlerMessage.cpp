#include "Kaleido3D.h"
#include "Core/Thread.h"

using namespace k3d;
int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int)
{
Thread thr;
thr.Start([](const char * log)
{
 OutputDebugStringA(log);
}, "what's the hell?");
thr.Join();
}