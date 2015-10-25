#pragma once 
#include "RHI/D3D12/Public/D3D12RHIResource.h"

NS_K3D_D3D12_BEGIN

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256

// Various types of allocations may contain NULL pointers.  Check before dereferencing if you are unsure.
struct DynAlloc
{
	DynAlloc(GpuResource& BaseResource, size_t ThisOffset, size_t ThisSize)
		: Buffer(BaseResource), Offset(ThisOffset), Size(ThisSize) {}

	GpuResource& Buffer;	// The D3D buffer associated with this memory.
	size_t Offset;			// Offset from start of buffer resource
	size_t Size;			// Reserved size of this allocation
	void* DataPtr;			// The CPU-writeable address
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;	// The GPU-visible address
};

class LinearAllocationPage : public GpuResource
{
public:
	LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Usage) : GpuResource()
	{
		m_Resource.Attach(pResource);
		m_UsageState = Usage;
		m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();
		m_Resource->Map(0, nullptr, &m_CpuVirtualAddress);
	}

	~LinearAllocationPage()
	{
		m_Resource->Unmap(0, nullptr);
	}

	void* m_CpuVirtualAddress;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};

enum LinearAllocatorType
{
	kInvalidAllocator = -1,

	kGpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
	kCpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

	kNumAllocatorTypes
};

enum
{
	kGpuAllocatorPageSize = 0x10000,	// 64K
	kCpuAllocatorPageSize = 0x200000	// 2MB
};

class LinearAllocatorPageManager : public RHIRoot
{
public:

	LinearAllocatorPageManager();
	LinearAllocationPage* RequestPage(void);
	void DiscardPages(uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages);

	void Destroy(void) { m_PagePool.clear(); }

private:

	LinearAllocationPage* CreateNewPage(void);

	static LinearAllocatorType sm_AutoType;

	LinearAllocatorType m_AllocationType;
	std::vector<std::unique_ptr<LinearAllocationPage> > m_PagePool;
	std::queue<std::pair<uint64_t, LinearAllocationPage*> > m_RetiredPages;
	std::queue<LinearAllocationPage*> m_AvailablePages;
	std::mutex m_Mutex;
};

class LinearAllocator
{
public:

	LinearAllocator(LinearAllocatorType Type) : m_AllocationType(Type), m_PageSize(0), m_CurOffset(~0ull), m_CurPage(nullptr)
	{
		K3D_ASSERT(Type > kInvalidAllocator && Type < kNumAllocatorTypes);
		m_PageSize = (Type == kGpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
	}

	DynAlloc Allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

	void CleanupUsedPages(uint64_t FenceID);

	static void DestroyAll(void)
	{
		sm_PageManager[0].Destroy();
		sm_PageManager[1].Destroy();
	}

private:

	static LinearAllocatorPageManager sm_PageManager[2];

	LinearAllocatorType m_AllocationType;
	size_t m_PageSize;
	size_t m_CurOffset;
	LinearAllocationPage* m_CurPage;
	std::vector<LinearAllocationPage*> m_RetiredPages;
};

NS_K3D_D3D12_END