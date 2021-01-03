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
#if _WIN32
        return _aligned_malloc(size, align);
#else
		void* res = nullptr;
		posix_memalign(&res, align, size);
		return res;
#endif
    }

    void* Allocator::realloc(void* origin, size_t size, size_t align, VkSystemAllocationScope scope)
    {
#if _WIN32
        return _aligned_realloc(origin, size, align);
#else
		return nullptr;
#endif
    }

    void Allocator::free(void* ptr)
    {
#if _WIN32
        _aligned_free(ptr);
#else
		free(ptr);
#endif
    }

    void Allocator::notifyAlloc(size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
    }

    void Allocator::notifyFree(size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
    {
    }

    GpuAllocator::GpuAllocator(GpuDevice* device)
		: device_(device)
    {
    }

    GpuAllocator::~GpuAllocator()
    {
    }

	uint32_t GpuAllocator::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, bool& memTypeFound)
	{
		for (uint32_t i = 0; i < device_mem_props_.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((device_mem_props_.memoryTypes[i].propertyFlags & properties) == properties)
				{
					memTypeFound = true;
					return i;
				}
			}
			typeBits >>= 1;
		}
		memTypeFound = false;
		return 0;
	}

	ngfx::Result GpuAllocator::allocateForBuffer(VkBuffer buffer, ngfx::StorageMode mode, MemoryItem& memItem)
	{
		VkMemoryRequirements memReq = {};
		device_->getBufferMemoryRequirements(buffer, &memReq);
		memReq.alignment;
		bool found = false;
		//VkExportMemoryAllocateInfoNV Info;
		VkMemoryAllocateInfo alloc_info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
			memReq.size,
			getMemoryTypeIndex(
				memReq.memoryTypeBits, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, found)
		};
		VkDeviceMemory memory = VK_NULL_HANDLE;
		device_->allocateMemory(&alloc_info, &memory);
		memItem.mem = memory;
		memItem.offset = 0u;

		return ngfx::Result::Ok;
	}

	ngfx::Result GpuAllocator::allocateForImage(VkImage image, ngfx::StorageMode mode, MemoryItem& memItem)
	{
		VkMemoryRequirements memReq = {};
		device_->getImageMemoryRequirements(image, &memReq);
		memReq.alignment;
		bool found = false;
		VkMemoryAllocateInfo alloc_info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
			memReq.size,
			getMemoryTypeIndex(
				memReq.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, found)
		};
		VkDeviceMemory memory = VK_NULL_HANDLE;
		device_->allocateMemory(&alloc_info, &memory);
		memItem.mem = memory;
		memItem.offset = 0u;

		return ngfx::Result::Ok;
	}

	ngfx::Result GpuAllocator::allocateForAccelerationStructure(VkAccelerationStructureNV accelerationStructure, ngfx::StorageMode mode, MemoryItem& memItem)
	{
		VkMemoryRequirements2 memoryRequirements2{};
		device_->getAccelerationStructureMemoryRequirements(accelerationStructure, 
			VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV, 
			&memoryRequirements2);
		memoryRequirements2.memoryRequirements.alignment;
		bool found = false;
		VkMemoryAllocateInfo alloc_info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
			memoryRequirements2.memoryRequirements.size,
			getMemoryTypeIndex(
				memoryRequirements2.memoryRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, found)
		};
		VkDeviceMemory memory = VK_NULL_HANDLE;
		device_->allocateMemory(&alloc_info, &memory);
		memItem.mem = memory;
		memItem.offset = 0u;

		return ngfx::Result::Ok;
	}

	void GpuAllocator::freeBuffer(VkBuffer buffer, MemoryItem const& item)
	{
		device_->freeMemory(item.mem);
	}

	void GpuAllocator::freeImage(VkImage image, MemoryItem const& item)
	{
		device_->freeMemory(item.mem);
	}

	void GpuAllocator::freeAccelerationStructure(VkAccelerationStructureNV accelerationStructure, MemoryItem const& item)
	{
		device_->freeMemory(item.mem);
	}

	void GpuAllocator::init()
	{
		device_->getPhysicalDeviceMemoryProperties(device_mem_props_);
	}
}