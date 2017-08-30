#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "Math/kMath.hpp"

NS_K3D_D3D12_BEGIN

using namespace std;

LinearAllocatorType LinearAllocatorPageManager::sm_AutoType = kGpuExclusive;

LinearAllocatorPageManager::LinearAllocatorPageManager(Device::Ptr pDevice)
	: D3D12RHIDeviceChild(pDevice)
{
	m_AllocationType = sm_AutoType;
	sm_AutoType = (LinearAllocatorType)(sm_AutoType + 1);
	K3D_ASSERT(sm_AutoType <= kNumAllocatorTypes);
}

LinearAllocationPage* LinearAllocatorPageManager::RequestPage()
{
	lock_guard<mutex> LockGuard(m_Mutex);

	while (!m_RetiredPages.empty() /*&& GetPrimaryCommandListManager().IsFenceComplete(m_RetiredPages.front().first)*/)
	{
		m_AvailablePages.push(m_RetiredPages.front().second);
		m_RetiredPages.pop();
	}

	LinearAllocationPage* PagePtr = nullptr;

	if (!m_AvailablePages.empty())
	{
		PagePtr = m_AvailablePages.front();
		m_AvailablePages.pop();
	}
	else
	{
		PagePtr = CreateNewPage();
		m_PagePool.emplace_back(PagePtr);
	}

	return PagePtr;
}

void LinearAllocatorPageManager::DiscardPages(uint64_t FenceValue, const vector<LinearAllocationPage*>& UsedPages)
{
	lock_guard<mutex> LockGuard(m_Mutex);
	for (auto iter = UsedPages.begin(); iter != UsedPages.end(); ++iter)
		m_RetiredPages.push(make_pair(FenceValue, *iter));
}

LinearAllocationPage* LinearAllocatorPageManager::CreateNewPage(void)
{
	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	D3D12_RESOURCE_STATES DefaultUsage;

	if (m_AllocationType == kGpuExclusive)
	{
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		ResourceDesc.Width = kGpuAllocatorPageSize;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		DefaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	else
	{
		HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		ResourceDesc.Width = kCpuAllocatorPageSize;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		DefaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	ID3D12Resource* pBuffer;
	ThrowIfFailed(GetParentDeviceRef().Get()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
		DefaultUsage, nullptr, IID_PPV_ARGS(&pBuffer)));

	pBuffer->SetName(L"LinearAllocator Page");

	return new LinearAllocationPage(GetParentDevicePtr(), pBuffer, DefaultUsage);
}

void LinearAllocator::CleanupUsedPages(uint64_t FenceID)
{
	if (m_CurPage == nullptr)
		return;

	m_RetiredPages.push_back(m_CurPage);
	m_CurPage = nullptr;
	m_CurOffset = 0;

	sm_PageManager[m_AllocationType]->DiscardPages(FenceID, m_RetiredPages);
	m_RetiredPages.clear();
}

DynAlloc LinearAllocator::Allocate(size_t SizeInBytes, size_t Alignment)
{
	K3D_ASSERT(SizeInBytes <= m_PageSize, "Exceeded max linear allocator page size with single allocation");

	const size_t AlignmentMask = Alignment - 1;

	// Assert that it's a power of two.
	K3D_ASSERT((AlignmentMask & Alignment) == 0);

	// Align the allocation
	const size_t AlignedSize = kMath::AlignUpWithMask(SizeInBytes, AlignmentMask);

	m_CurOffset = kMath::AlignUp(m_CurOffset, Alignment);

	if (m_CurOffset + AlignedSize > m_PageSize)
	{
		K3D_ASSERT(m_CurPage != nullptr);
		m_RetiredPages.push_back(m_CurPage);
		m_CurPage = nullptr;
	}

	if (m_CurPage == nullptr)
	{
		m_CurPage = sm_PageManager[m_AllocationType]->RequestPage();
		m_CurOffset = 0;
	}

	DynAlloc ret(*m_CurPage, m_CurOffset, AlignedSize);
	ret.DataPtr = (uint8_t*)m_CurPage->m_CpuVirtualAddress + m_CurOffset;
	ret.GpuAddress = m_CurPage->m_GpuVirtualAddress + m_CurOffset;

	m_CurOffset += AlignedSize;

	return ret;
}

NS_K3D_D3D12_END