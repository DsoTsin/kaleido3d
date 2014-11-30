#pragma once
#ifndef __DescriptorHeap_h__
#define __DescriptorHeap_h__

#include <mutex>
#include "RHI.h"
#include "Private/D3D12RootSignature.h"

NS_K3D_D3D12_BEGIN

/**
* DescriptorHandle from [MiniEngine Demo](Microsoft)
*/
class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		CpuHandle.ptr = ~0ull;
		GpuHandle.ptr = ~0ull;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle)
		: CpuHandle(CpuHandle)
	{
		GpuHandle.ptr = ~0ull;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
		: CpuHandle(CpuHandle), GpuHandle(GpuHandle)
	{
	}

	DescriptorHandle operator+ (ULONG OffsetScaledByDescriptorSize) const
	{
		DescriptorHandle ret = *this;
		ret += OffsetScaledByDescriptorSize;
		return ret;
	}

	void operator += (ULONG OffsetScaledByDescriptorSize)
	{
		if (CpuHandle.ptr != ~0ull)
			CpuHandle.ptr += OffsetScaledByDescriptorSize;
		if (GpuHandle.ptr != ~0ull)
			GpuHandle.ptr += OffsetScaledByDescriptorSize;
	}

	bool IsNull() const { return CpuHandle.ptr == ~0ull; }
	bool IsShaderVisible() const { return GpuHandle.ptr != ~0ull; }

	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
};

// <Named Parameter - Slot>
class DescriptorSlot {
public:
	uint32 Index; // Slot Index
	DescriptorHandle Handle; // Slot Handle
private:
	PtrDescHeap m_Heap;
	//std::vector<>
};

// This is an unbounded resource descriptor allocator.  It is intended to provide space for CPU-visible resource descriptors
// as resources are created.  For those that need to be made shader-visible, they will need to be copied to a UserDescriptorHeap
// or a DynamicDescriptorHeap.
class DescriptorAllocator : public RHIRoot
{
public:
	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) : m_Type(Type), m_CurrentHeap(nullptr) {}

	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t Count);

	static void DestroyAll(void);

protected:

	static const uint32_t sm_NumDescriptorsPerHeap = 256;
	static std::mutex sm_AllocationMutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool;
	static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);

	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	ID3D12DescriptorHeap* m_CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
	uint32_t m_DescriptorSize;
	uint32_t m_RemainingFreeHandles;
};

/**
 * \brief	A dynamic descriptor heap allocator,
 * 			for re-use of heaps
 */


class CommandContext;
// This class is a linear allocation system for dynamically generated descriptor tables.  It internally caches
// CPU descriptor handles so that when not enough space is available in the current heap, necessary descriptors
// can be re-copied to the new heap.
class DynamicDescriptorHeap : public RHIRoot
{
public:
	DynamicDescriptorHeap(CommandContext& OwningContext);
	~DynamicDescriptorHeap();

	static void DestroyAll(void);

	void CleanupUsedHeaps(uint64_t fenceValue);

	// Copy multiple handles into the cache area reserved for the specified root parameter.
	void SetGraphicsDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_GraphicsHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
	}

	void SetComputeDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_ComputeHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
	}

	// Bypass the cache and upload directly to the shader-visible heap
	D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handles);

	// Deduce cache layout needed to support the descriptor tables needed by the root signature.
	void ParseGraphicsRootSignature(const RootSignature& RootSig)
	{
		m_GraphicsHandleCache.ParseRootSignature(RootSig);
	}

	void ParseComputeRootSignature(const RootSignature& RootSig)
	{
		m_ComputeHandleCache.ParseRootSignature(RootSig);
	}

	// Upload any new descriptors in the cache to the shader-visible heap.
	inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* CmdList)
	{
		if (m_GraphicsHandleCache.m_StaleRootParamsBitMap != 0)
			CopyAndBindStagedTables(m_GraphicsHandleCache, CmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
	}

	inline void CommitComputeRootDescriptorTables(ID3D12GraphicsCommandList* CmdList)
	{
		if (m_ComputeHandleCache.m_StaleRootParamsBitMap != 0)
			CopyAndBindStagedTables(m_ComputeHandleCache, CmdList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
	}

	static uint32_t GetDescriptorSize()
	{
		if (sm_DescriptorSize == 0)
			sm_DescriptorSize = GetPrimaryD3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return sm_DescriptorSize;
	}

private:

	// Static members
	static const uint32_t kNumDescriptorsPerHeap = 1024;
	static std::mutex sm_Mutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool;
	static std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> sm_RetiredDescriptorHeaps;
	static std::queue<ID3D12DescriptorHeap*> sm_AvailableDescriptorHeaps;
	static uint32_t sm_DescriptorSize;

	// Static methods
	static ID3D12DescriptorHeap* RequestDescriptorHeap(void);
	static void DiscardDescriptorHeaps(uint64_t FenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps);

	// Non-static members
	CommandContext& m_OwningContext;
	ID3D12DescriptorHeap* m_CurrentHeapPtr;
	uint32_t m_CurrentOffset;
	DescriptorHandle m_FirstDescriptor;
	std::vector<ID3D12DescriptorHeap*> m_RetiredHeaps;

	// Describes a descriptor table entry:  a region of the handle cache and which handles have been set
	struct DescriptorTableCache
	{
		DescriptorTableCache() : AssignedHandlesBitMap(0) {}
		uint32_t AssignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE* TableStart;
		uint32_t TableSize;
	};

	struct DescriptorHandleCache
	{
		DescriptorHandleCache()
		{
			ClearCache();
		}

		void ClearCache()
		{
			m_RootDescriptorTablesBitMap = 0;
			m_MaxCachedDescriptors = 0;
		}

		uint32_t m_RootDescriptorTablesBitMap;
		uint32_t m_StaleRootParamsBitMap;
		uint32_t m_MaxCachedDescriptors;

		static const uint32_t kMaxNumDescriptors = 256;
		static const uint32_t kMaxNumDescriptorTables = 16;

		uint32_t ComputeStagedSize();
		void CopyAndBindStaleTables(DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
			void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

		DescriptorTableCache m_RootDescriptorTable[kMaxNumDescriptorTables];
		D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCache[kMaxNumDescriptors];

		void UnbindAllValid();
		void StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		void ParseRootSignature(const RootSignature& RootSig);
	};

	DescriptorHandleCache m_GraphicsHandleCache;
	DescriptorHandleCache m_ComputeHandleCache;

	bool HasSpace(uint32_t Count)
	{
		return (m_CurrentHeapPtr != nullptr && m_CurrentOffset + Count <= kNumDescriptorsPerHeap);
	}

	void RetireCurrentHeap(void);
	void RetireUsedHeaps(uint64_t fenceValue);
	ID3D12DescriptorHeap* GetHeapPointer();

	DescriptorHandle Allocate(UINT Count)
	{
		DescriptorHandle ret = m_FirstDescriptor + m_CurrentOffset * GetDescriptorSize();
		m_CurrentOffset += Count;
		return ret;
	}

	void CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
		void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

	// Mark all descriptors in the cache as stale and in need of re-uploading.
	void UnbindAllValid(void);

};

namespace ModuleD3D12
{
	extern DescriptorAllocator g_DescriptorAllocator[];
	inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1)
	{
		return g_DescriptorAllocator[Type].Allocate(Count);
	}
}

NS_K3D_D3D12_END

#endif