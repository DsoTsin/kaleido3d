#include "Kaleido3D.h"
#include <KTL/Allocator.hpp>

K3D_API void* __k3d_malloc__(size_t sizeOfObj)
{
	return malloc(sizeOfObj);
}

K3D_API void __k3d_free__(void *p, size_t sizeOfObj)
{
	free(p);
}

K3D_API void* operator new[](size_t size, const char* pName)
{
	return malloc(size);
}
