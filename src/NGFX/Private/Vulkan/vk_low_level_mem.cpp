#include "vk_common.h"

namespace vulkan {
    Allocator               gAllocator;
    VkAllocationCallbacks   gAllocationCallbacks = {
        &gAllocator,
        &Allocator::alloc,
        &Allocator::realloc,
        &Allocator::free,

        &Allocator::notifyAlloc,
        &Allocator::notifyFree,
    };

    void * Allocator::alloc(void* usr, size_t size, size_t align, VkSystemAllocationScope scope)
    {
        Allocator* allocator = (Allocator*)usr;
        return allocator->alloc(size, align, scope);
    }

    void * Allocator::realloc(void * usr, void * origin, size_t size, size_t align, VkSystemAllocationScope scope)
    {
        Allocator* allocator = (Allocator*)usr;
        return allocator->realloc(origin, size, align, scope);
    }

    void Allocator::free(void* usr, void * ptr)
    {
        Allocator* allocator = (Allocator*)usr;
        allocator->free(ptr);
    }

    void Allocator::notifyAlloc(void* usr, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
        Allocator* allocator = (Allocator*)usr;
        allocator->notifyAlloc(size, type, scope);
    }

    void Allocator::notifyFree(void * usr, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
        Allocator* allocator = (Allocator*)usr;
        allocator->notifyFree(size, type, scope);
    }

    Allocator::Allocator()
    {
    }

    Allocator::~Allocator()
    {
    }

    void* Allocator::alloc(size_t size, size_t align, VkSystemAllocationScope scope)
    {
        return _aligned_malloc(size, align);
    }

    void* Allocator::realloc(void* origin, size_t size, size_t align, VkSystemAllocationScope scope)
    {
        return _aligned_realloc(origin, size, align);
    }

    void Allocator::free(void* ptr)
    {
        _aligned_free(ptr);
    }

    void Allocator::notifyAlloc(size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
    }

    void Allocator::notifyFree(size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
    }

}