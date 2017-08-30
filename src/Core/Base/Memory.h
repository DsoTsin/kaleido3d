#pragma once

namespace k3d
{
    extern K3D_CORE_API void MemoryCopy(void* Dest, void* Src, size_t Size);
    extern K3D_CORE_API void MemoryFill(void* Dest, int Value, size_t Size);
}