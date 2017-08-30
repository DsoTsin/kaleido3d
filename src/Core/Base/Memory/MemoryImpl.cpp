#include "CoreMinimal.h"

#include <string.h>

namespace k3d
{
    void MemoryCopy(void* Dest, void* Src, size_t Size)
    {
        memcpy(Dest, Src, Size);
    }

    void MemoryFill(void* Dest, int Value, size_t Size)
    {
        memset(Dest, Value, Size);
    }
}