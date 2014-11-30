#include "DXCommon.h"
#include "D3D12CommandListManager.h"

NS_K3D_D3D12_BEGIN

DirectCommandListManager::DirectCommandListManager() : m_Device(nullptr), m_CommandQueue(nullptr)
{
}

DirectCommandListManager::~DirectCommandListManager()
{
	Shutdown();
}

void DirectCommandListManager::Shutdown()
{
	if (m_CommandQueue == nullptr)
		return;

	CloseHandle(m_FenceEventHandle);

	for (size_t i = 0; i < m_AllocatorPool.size(); ++i)
		m_AllocatorPool[i]->Release();

	m_pFence->Release();

	m_CommandQueue->Release();
	m_CommandQueue = nullptr;
}

void DirectCommandListManager::Create(PtrDevice pDevice)
{
	K3D_ASSERT(pDevice != nullptr);
	K3D_ASSERT(!IsReady());
	K3D_ASSERT(m_AllocatorPool.size() == 0);

	m_Device = pDevice;

#ifndef RELEASE
	// Prevent the GPU from overclocking or underclocking to get consistent timings
	pDevice->SetStablePowerState(TRUE);
#endif

	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.NodeMask = 1;
	m_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue));
	m_CommandQueue->SetName(L"DirectCommandListManager::m_CommandQueue");

	m_NextFenceValue = 1;
	m_LastCompletedFenceValue = 0;
	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
	m_pFence->SetName(L"DirectCommandListManager::m_pFence");
	m_pFence->Signal(0);

	m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
	K3D_ASSERT(m_FenceEventHandle != INVALID_HANDLE_VALUE);

	K3D_ASSERT(IsReady());
}

void DirectCommandListManager::CreateNewCommandList(ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator)
{
	*Allocator = RequestAllocator();
	ThrowIfFailed(m_Device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, *Allocator, nullptr, IID_PPV_ARGS(List)));
	(*List)->SetName(L"CommandList");
}

uint64_t DirectCommandListManager::ExecuteCommandList(ID3D12CommandList* List)
{
	std::lock_guard<std::mutex> LockGuard(m_FenceMutex);

	// Kickoff the command list
	m_CommandQueue->ExecuteCommandLists(1, &List);

	// Signal the next fence value (with the GPU)
	m_CommandQueue->Signal(m_pFence, m_NextFenceValue);

	// And increment the fence value.  
	return m_NextFenceValue++;
}

ID3D12CommandAllocator* DirectCommandListManager::RequestAllocator(void)
{
	std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);

	ID3D12CommandAllocator* pAllocator = nullptr;

	if (!m_ReadyAllocators.empty())
	{
		std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocators.front();

		if (IsFenceComplete(AllocatorPair.first))
		{
			pAllocator = AllocatorPair.second;
			ThrowIfFailed(pAllocator->Reset());
			m_ReadyAllocators.pop();
		}
	}

	// If no allocator's were ready to be reused, create a new one
	if (pAllocator == nullptr)
	{
		ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pAllocator)));
		wchar_t AllocatorName[32];
		swprintf(AllocatorName, 32, L"CommandAllocator %zu", m_AllocatorPool.size());
		pAllocator->SetName(AllocatorName);
		m_AllocatorPool.push_back(pAllocator);
	}

	return pAllocator;
}

void DirectCommandListManager::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
{
	std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);

	// That fence value indicates we are free to reset the allocator
	m_ReadyAllocators.push(std::make_pair(FenceValue, Allocator));
}

uint64_t DirectCommandListManager::IncrementFence(void)
{
	std::lock_guard<std::mutex> LockGuard(m_FenceMutex);
	m_CommandQueue->Signal(m_pFence, m_NextFenceValue);
	return m_NextFenceValue++;
}

bool DirectCommandListManager::IsFenceComplete(uint64_t FenceValue)
{
	// Avoid querying the fence value by testing against the last one seen.
	// The max() is to protect against an unlikely race condition that could cause the last
	// completed fence value to regress.
	if (FenceValue > m_LastCompletedFenceValue)
		m_LastCompletedFenceValue = max(m_LastCompletedFenceValue, m_pFence->GetCompletedValue());

	return FenceValue <= m_LastCompletedFenceValue;
}


void DirectCommandListManager::WaitForFence(uint64_t FenceValue)
{
	if (IsFenceComplete(FenceValue))
		return;

	// TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
	// wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
	// the fence can only have one event set on completion, then thread B has to wait for 
	// 100 before it knows 99 is ready.  Maybe insert sequential events?
	{
		std::lock_guard<std::mutex> LockGuard(m_EventMutex);

		m_pFence->SetEventOnCompletion(FenceValue, m_FenceEventHandle);
		WaitForSingleObject(m_FenceEventHandle, INFINITE);
		m_LastCompletedFenceValue = FenceValue;
	}
}


NS_K3D_D3D12_END