#pragma once 

NS_K3D_D3D12_BEGIN

class DirectCommandListManager
{
	friend class CommandContext;

public:
	DirectCommandListManager();
	~DirectCommandListManager();

	void Create(PtrDevice pDevice);
	void Shutdown();

	inline bool IsReady()
	{
		return m_CommandQueue != nullptr;
	}

	uint64_t IncrementFence(void);
	bool IsFenceComplete(uint64_t FenceValue);
	void WaitForFence(uint64_t FenceValue);
	void IdleGPU(void) { WaitForFence(IncrementFence()); }

	ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }
	PtrDevice GetRHIDevice() { return m_Device; }

private:

	void CreateNewCommandList(ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator);
	uint64_t ExecuteCommandList(ID3D12CommandList* List);
	ID3D12CommandAllocator* RequestAllocator(void);
	void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

	PtrDevice m_Device;
	ID3D12CommandQueue* m_CommandQueue;

	// Since there is only a "main pool" so far, everything below corresponds to that pool. It should be renamed and/or 
	// restructured if we add other pools.
	std::vector<ID3D12CommandAllocator*> m_AllocatorPool;
	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
	std::mutex m_AllocatorMutex;
	std::mutex m_FenceMutex;
	std::mutex m_EventMutex;

	// Lifetime of these objects is managed by the descriptor cache
	ID3D12Fence* m_pFence;
	uint64_t m_NextFenceValue;
	uint64_t m_LastCompletedFenceValue;
	HANDLE m_FenceEventHandle;
};


NS_K3D_D3D12_END