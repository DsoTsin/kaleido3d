#include "Kaleido3D.h"
#include <Core/Os.h>

namespace Os
{
float* GetCpuUsage()
{
	return nullptr;
}

uint32
GetGpuCount()
{
  return 1;
}

float GetGpuUsage(int Id)
{
  return 0.1f;
}
}
