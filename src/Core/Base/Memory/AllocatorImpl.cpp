#include "CoreMinimal.h"
#include <stdlib.h>

namespace k3d
{
    class SystemAllocator : public IAllocatorAdapter
    {
    public:
        SystemAllocator()
        {}
        ~SystemAllocator()
        {}
        const char* GetName() const override { return "LibCAllocator"; }
        void* Alloc(size_t SzToAlloc, int Alignment, int AlignOffset, int Flags, const char* AllocInfo) override;
        void  DeAlloc(void* Ptr) override;
    };

    void * SystemAllocator::Alloc(size_t SzToAlloc, int Alignment, int AlignOffset, int Flags, const char * AllocInfo)
    {
        if (Alignment == 0)
            return malloc(SzToAlloc);
        else
#if K3DPLATFORM_OS_WINDOWS
            return _aligned_malloc(SzToAlloc, Alignment);
#else
        {
            void *pAddr = nullptr;
            posix_memalign(&pAddr, Alignment, SzToAlloc);
            return pAddr;
        }
#endif
    }

    void SystemAllocator::DeAlloc(void * Ptr)
    {
        free(Ptr);
    }

    IAllocatorAdapter& GetDefaultAllocator()
    {
        static SystemAllocator SysAllc;
        return SysAllc;
    }
}

K3D_CORE_API void* k3d_malloc(size_t SzObj)
{
	return k3d::GetDefaultAllocator().Alloc(SzObj);
}

K3D_CORE_API void* k3d_malloc_aligned(size_t SzObj, int Align)
{
    return k3d::GetDefaultAllocator().Alloc(SzObj, Align);
}

K3D_CORE_API void k3d_free(void *p, size_t sizeOfObj)
{
    return k3d::GetDefaultAllocator().DeAlloc(p);
}

K3D_CORE_API void* operator new[](size_t size, const char* pName)
{
	return k3d_malloc(size);
}

K3D_CORE_API void* operator new(size_t Size, const char* _ClassName, const char* _SourceFile, int _SourceLine)
{
  return k3d_malloc(Size);
}

K3D_CORE_API void operator delete(void* _Ptr, const char* _SourceFile, int _SourceLine)
{
    k3d_free(_Ptr, 0);
}
