#include "DXCommon.h"
#include "DescriptorHeap.h"
#include "Public/D3D12RHI.h"
#include "Core/LogUtil.h"

NS_K3D_D3D12_BEGIN

//
// DescriptorAllocator implementation
//
std::mutex DescriptorAllocator::sm_AllocationMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sm_DescriptorHeapPool;

void DescriptorAllocator::DestroyAll(void)
{
	sm_DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(sm_AllocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = sm_NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;
	ComPtr<ID3D12DescriptorHeap> pHeap;
	ThrowIfFailed(GetPrimaryD3DDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap)));
	sm_DescriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t Count)
{
	if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < Count)
	{
		m_CurrentHeap = RequestNewHeap(m_Type);
		m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
		m_RemainingFreeHandles = sm_NumDescriptorsPerHeap;

		if (m_DescriptorSize == 0)
			m_DescriptorSize = GetPrimaryD3DDevice()->GetDescriptorHandleIncrementSize(m_Type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
	m_CurrentHandle.ptr += Count * m_DescriptorSize;
	m_RemainingFreeHandles -= Count;
	return ret;
}

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)

//
// DynamicDescriptorHeap Implementation
//

std::mutex DynamicDescriptorHeap::sm_Mutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DynamicDescriptorHeap::sm_DescriptorHeapPool;
std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> DynamicDescriptorHeap::sm_RetiredDescriptorHeaps;
std::queue<ID3D12DescriptorHeap*> DynamicDescriptorHeap::sm_AvailableDescriptorHeaps;
uint32_t DynamicDescriptorHeap::sm_DescriptorSize = 0;

void DynamicDescriptorHeap::DestroyAll(void)
{
	sm_DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap(void)
{
	std::lock_guard<std::mutex> LockGuard(sm_Mutex);

	while (!sm_RetiredDescriptorHeaps.empty() && GetPrimaryCommandListManager().IsFenceComplete(sm_RetiredDescriptorHeaps.front().first))
	{
		sm_AvailableDescriptorHeaps.push(sm_RetiredDescriptorHeaps.front().second);
		sm_RetiredDescriptorHeaps.pop();
	}

	if (!sm_AvailableDescriptorHeaps.empty())
	{
		ID3D12DescriptorHeap* HeapPtr = sm_AvailableDescriptorHeaps.front();
		sm_AvailableDescriptorHeaps.pop();
		return HeapPtr;
	}
	else
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HeapDesc.NumDescriptors = kNumDescriptorsPerHeap;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HeapDesc.NodeMask = 1;
		ComPtr<ID3D12DescriptorHeap> HeapPtr;
		ThrowIfFailed(GetPrimaryD3DDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&HeapPtr)));
		sm_DescriptorHeapPool.emplace_back(HeapPtr);
		return HeapPtr.Get();
	}
}

void DynamicDescriptorHeap::DiscardDescriptorHeaps(uint64_t FenceValue, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps)
{
	std::lock_guard<std::mutex> LockGuard(sm_Mutex);
	for (auto iter = UsedHeaps.begin(); iter != UsedHeaps.end(); ++iter)
		sm_RetiredDescriptorHeaps.push(std::make_pair(FenceValue, *iter));
}

void DynamicDescriptorHeap::RetireCurrentHeap(void)
{
	// Don't retire unused heaps.
	if (m_CurrentOffset == 0)
	{
		K3D_ASSERT(m_CurrentHeapPtr == nullptr);
		return;
	}

	K3D_ASSERT(m_CurrentHeapPtr != nullptr);
	m_RetiredHeaps.push_back(m_CurrentHeapPtr);
	m_CurrentHeapPtr = nullptr;
	m_CurrentOffset = 0;
}

void DynamicDescriptorHeap::RetireUsedHeaps(uint64_t fenceValue)
{
	DiscardDescriptorHeaps(fenceValue, m_RetiredHeaps);
	m_RetiredHeaps.clear();
}

DynamicDescriptorHeap::DynamicDescriptorHeap(CommandContext& OwningContext) : m_OwningContext(OwningContext)
{
	m_CurrentHeapPtr = nullptr;
	m_CurrentOffset = 0;
}

DynamicDescriptorHeap::~DynamicDescriptorHeap()
{
}

void DynamicDescriptorHeap::CleanupUsedHeaps(uint64_t fenceValue)
{
	RetireCurrentHeap();
	RetireUsedHeaps(fenceValue);
	m_GraphicsHandleCache.ClearCache();
	m_ComputeHandleCache.ClearCache();
}

inline ID3D12DescriptorHeap* DynamicDescriptorHeap::GetHeapPointer()
{
	if (m_CurrentHeapPtr == nullptr)
	{
		K3D_ASSERT(m_CurrentOffset == 0);
		m_CurrentHeapPtr = RequestDescriptorHeap();
		m_FirstDescriptor = DescriptorHandle(
			m_CurrentHeapPtr->GetCPUDescriptorHandleForHeapStart(),
			m_CurrentHeapPtr->GetGPUDescriptorHandleForHeapStart());
	}

	return m_CurrentHeapPtr;
}

uint32_t DynamicDescriptorHeap::DescriptorHandleCache::ComputeStagedSize()
{
	// Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
	uint32_t NeededSpace = 0;
	uint32_t RootIndex;
	uint32_t StaleParams = m_StaleRootParamsBitMap;
	while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
	{
		StaleParams ^= (1 << RootIndex);

		uint32_t MaxSetHandle;
		K3D_ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
			"Root entry marked as stale but has no stale descriptors");

		NeededSpace += MaxSetHandle + 1;
	}
	return NeededSpace;
}

void DynamicDescriptorHeap::DescriptorHandleCache::CopyAndBindStaleTables(
	DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
	void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t StaleParamCount = 0;
	uint32_t TableSize[DescriptorHandleCache::kMaxNumDescriptorTables];
	uint32_t RootIndices[DescriptorHandleCache::kMaxNumDescriptorTables];
	uint32_t NeededSpace = 0;
	uint32_t RootIndex;

	// Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
	uint32_t StaleParams = m_StaleRootParamsBitMap;
	while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
	{
		RootIndices[StaleParamCount] = RootIndex;
		StaleParams ^= (1 << RootIndex);

		uint32_t MaxSetHandle;
		K3D_ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
			"Root entry marked as stale but has no stale descriptors");

		NeededSpace += MaxSetHandle + 1;
		TableSize[StaleParamCount] = MaxSetHandle + 1;

		++StaleParamCount;
	}

	K3D_ASSERT(StaleParamCount <= DescriptorHandleCache::kMaxNumDescriptorTables,
		"We're only equipped to handle so many descriptor tables");

	m_StaleRootParamsBitMap = 0;

	static const uint32_t kMaxDescriptorsPerCopy = 16;
	UINT NumDestDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[kMaxDescriptorsPerCopy];
	UINT pDestDescriptorRangeSizes[kMaxDescriptorsPerCopy];

	UINT NumSrcDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE pSrcDescriptorRangeStarts[kMaxDescriptorsPerCopy];
	UINT pSrcDescriptorRangeSizes[kMaxDescriptorsPerCopy];

	const uint32_t kDescriptorSize = DynamicDescriptorHeap::GetDescriptorSize();

	for (uint32_t i = 0; i < StaleParamCount; ++i)
	{
		RootIndex = RootIndices[i];
		(CmdList->*SetFunc)(RootIndex, DestHandleStart.GpuHandle);

		DescriptorTableCache& RootDescTable = m_RootDescriptorTable[RootIndex];

		D3D12_CPU_DESCRIPTOR_HANDLE* SrcHandles = RootDescTable.TableStart;
		uint64_t SetHandles = (uint64_t)RootDescTable.AssignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE CurDest = DestHandleStart.CpuHandle;
		DestHandleStart += TableSize[i] * kDescriptorSize;

		unsigned long SkipCount;
		while (_BitScanForward64(&SkipCount, SetHandles))
		{
			// Skip over unset descriptor handles
			SetHandles >>= SkipCount;
			SrcHandles += SkipCount;
			CurDest.ptr += SkipCount * kDescriptorSize;

			unsigned long DescriptorCount;
			_BitScanForward64(&DescriptorCount, ~SetHandles);
			SetHandles >>= DescriptorCount;

			// If we run out of temp room, copy what we've got so far
			if (NumSrcDescriptorRanges + DescriptorCount > kMaxDescriptorsPerCopy)
			{
				GetPrimaryD3DDevice()->CopyDescriptors(
					NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
					NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				NumSrcDescriptorRanges = 0;
				NumDestDescriptorRanges = 0;
			}

			// Setup destination range
			pDestDescriptorRangeStarts[NumDestDescriptorRanges] = CurDest;
			pDestDescriptorRangeSizes[NumDestDescriptorRanges] = DescriptorCount;
			++NumDestDescriptorRanges;

			// Setup source ranges (one descriptor each because we don't assume they are contiguous)
			for (UINT _i = 0; _i < DescriptorCount; ++_i)
			{
				pSrcDescriptorRangeStarts[NumSrcDescriptorRanges] = SrcHandles[_i];
				pSrcDescriptorRangeSizes[NumSrcDescriptorRanges] = 1;
				++NumSrcDescriptorRanges;
			}

			// Move the destination pointer forward by the number of descriptors we will copy
			SrcHandles += DescriptorCount;
			CurDest.ptr += DescriptorCount * kDescriptorSize;
		}
	}

	GetPrimaryD3DDevice()->CopyDescriptors(
		NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
		NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DynamicDescriptorHeap::CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
	void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t NeededSize = HandleCache.ComputeStagedSize();
	if (!HasSpace(NeededSize))
	{
		RetireCurrentHeap();
		UnbindAllValid();
	}

	// This can trigger the creation of a new heap
	m_OwningContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GetHeapPointer());

	HandleCache.CopyAndBindStaleTables(Allocate(NeededSize), CmdList, SetFunc);
}

void DynamicDescriptorHeap::UnbindAllValid(void)
{
	m_GraphicsHandleCache.UnbindAllValid();
	m_ComputeHandleCache.UnbindAllValid();
}

D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
	if (!HasSpace(1))
	{
		RetireCurrentHeap();
		UnbindAllValid();
	}

	m_OwningContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GetHeapPointer());

	DescriptorHandle DestHandle = m_FirstDescriptor + m_CurrentOffset * GetDescriptorSize();
	m_CurrentOffset += 1;

	GetPrimaryD3DDevice()->CopyDescriptorsSimple(1, DestHandle.CpuHandle, Handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return DestHandle.GpuHandle;
}

void DynamicDescriptorHeap::DescriptorHandleCache::UnbindAllValid()
{
	m_StaleRootParamsBitMap = 0;

	unsigned long TableParams = m_RootDescriptorTablesBitMap;
	unsigned long RootIndex;
	while (_BitScanForward(&RootIndex, TableParams))
	{
		TableParams ^= (1 << RootIndex);
		if (m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap != 0)
			m_StaleRootParamsBitMap |= (1 << RootIndex);
	}
}

void DynamicDescriptorHeap::DescriptorHandleCache::StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
{
	K3D_ASSERT(((1 << RootIndex) & m_RootDescriptorTablesBitMap) != 0, "Root parameter is not a CBV_SRV_UAV descriptor table");
	K3D_ASSERT(Offset + NumHandles <= m_RootDescriptorTable[RootIndex].TableSize);

	DescriptorTableCache& TableCache = m_RootDescriptorTable[RootIndex];
	D3D12_CPU_DESCRIPTOR_HANDLE* CopyDest = TableCache.TableStart + Offset;
	for (UINT i = 0; i < NumHandles; ++i)
		CopyDest[i] = Handles[i];
	TableCache.AssignedHandlesBitMap |= ((1 << NumHandles) - 1) << Offset;
	m_StaleRootParamsBitMap |= (1 << RootIndex);
}

void DynamicDescriptorHeap::DescriptorHandleCache::ParseRootSignature(const RootSignature& RootSig)
{
	UINT CurrentOffset = 0;

	K3D_ASSERT(RootSig.m_NumParameters <= 16, "Maybe we need to support something greater");

	m_StaleRootParamsBitMap = 0;
	m_RootDescriptorTablesBitMap = RootSig.m_DescriptorTableBitMap;

	unsigned long TableParams = m_RootDescriptorTablesBitMap;
	unsigned long RootIndex;
	while (_BitScanForward(&RootIndex, TableParams))
	{
		TableParams ^= (1 << RootIndex);

		UINT TableSize = RootSig.m_DescriptorTableSize[RootIndex];
		K3D_ASSERT(TableSize > 0);

		DescriptorTableCache& RootDescriptorTable = m_RootDescriptorTable[RootIndex];
		RootDescriptorTable.AssignedHandlesBitMap = 0;
		RootDescriptorTable.TableStart = m_HandleCache + CurrentOffset;
		RootDescriptorTable.TableSize = TableSize;

		CurrentOffset += TableSize;
	}

	m_MaxCachedDescriptors = CurrentOffset;

	K3D_ASSERT(m_MaxCachedDescriptors <= kMaxNumDescriptors, "Exceeded user-supplied maximum cache size");
}

namespace ModuleD3D12
{
	DescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};
}

NS_K3D_D3D12_END