#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "Private/D3D12Enums.h"

NS_K3D_D3D12_BEGIN

CommandContext::CommandContext() :
	m_DynamicDescriptorHeap(*this),
	m_CpuLinearAllocator(kCpuWritable),
	m_GpuLinearAllocator(kGpuExclusive)
{
	m_OwningManager = nullptr;
	m_CommandList = nullptr;
	m_CurrentAllocator = nullptr;
	ZeroMemory(m_CurrentDescriptorHeaps, sizeof(m_CurrentDescriptorHeaps));
	m_CurGraphicsRootSignature = nullptr;
	m_CurGraphicsPipelineState = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurComputePipelineState = nullptr;
	m_NumBarriersToFlush = 0;
}

CommandContext::~CommandContext()
{

}

void CommandContext::Initialize()
{
	K3D_ASSERT(m_OwningManager != nullptr);
	m_OwningManager->CreateNewCommandList(&m_CommandList, &m_CurrentAllocator);
}

void CommandContext::Detach(rhi::IDevice *)
{

}

void CommandContext::CopyBuffer(rhi::IGpuResource & Dest, rhi::IGpuResource & Src)
{
}

void CommandContext::SetIndexBuffer(const rhi::IndexBufferView & IBView)
{
	const IndexBufferView & IBV = reinterpret_cast<const IndexBufferView&>(IBView);

}

void CommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView & VBView)
{
}

void CommandContext::SetPipelineState(uint32 Hash, rhi::IPipelineState *RhiPipeLineState)
{
	PipelineState * pPSO = static_cast<PipelineState*>(RhiPipeLineState);
	K3D_ASSERT(pPSO != nullptr);
	//? pPSO->Finalize();
	m_CommandList->SetPipelineState(pPSO->GetPipelineStateObject());
}

void CommandContext::SetViewport(const rhi::Viewport& Vp)
{
	m_CommandList->RSSetViewports(1, (const D3D12_VIEWPORT*) &Vp);
}

void CommandContext::SetPrimitiveType(rhi::EPrimitiveType PrimType)
{
	m_CommandList->IASetPrimitiveTopology(g_PrimitiveTopology[PrimType]);
}

void CommandContext::DrawInstanced(rhi::DrawInstanceParam Param)
{
	FlushResourceBarriers();
	m_CommandList->DrawInstanced(
		Param.VertexCountPerInstance, Param.InstanceCount, 
		Param.StartVertexLocation, Param.StartInstanceLocation);
}

void CommandContext::DrawIndexedInstanced(rhi::DrawIndexedInstancedParam Param)
{
	FlushResourceBarriers();
	m_CommandList->DrawIndexedInstanced(
		Param.IndexCountPerInstance, Param.InstanceCount, 
		Param.StartIndexLocation, Param.BaseVertexLocation, Param.StartInstanceLocation);
}

void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr)
{

}

void CommandContext::SetDynamicVB(UINT Slot, size_t NumVertices, size_t VertexStride, const void * VBData)
{
}

void CommandContext::SetDynamicIB(size_t IndexCount, const uint16_t * IBData)
{
}

void CommandContext::Reset()
{
	K3D_ASSERT(m_CommandList != nullptr && m_CurrentAllocator == nullptr);
	m_CurrentAllocator = m_OwningManager->RequestAllocator();
	m_CommandList->Reset(m_CurrentAllocator, nullptr);

	m_CurGraphicsRootSignature = nullptr;
	m_CurGraphicsPipelineState = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurComputePipelineState = nullptr;
	m_NumBarriersToFlush = 0;
}

void CommandContext::Execute(bool Wait)
{
	FlushResourceBarriers();

	K3D_ASSERT(m_CurrentAllocator != nullptr);
	K3D_ASSERT(SUCCEEDED(m_CommandList->Close()));

	uint64_t FenceValue = m_OwningManager->ExecuteCommandList(m_CommandList);
	m_OwningManager->DiscardAllocator(FenceValue, m_CurrentAllocator);
	m_CurrentAllocator = nullptr;

	m_CpuLinearAllocator.CleanupUsedPages(FenceValue);
	m_GpuLinearAllocator.CleanupUsedPages(FenceValue);
	m_DynamicDescriptorHeap.CleanupUsedHeaps(FenceValue);

	if (Wait)
		m_OwningManager->WaitForFence(FenceValue);

	//return FenceValue;
}

void CommandContext::FlushResourceBarriers()
{
	if (m_NumBarriersToFlush == 0)
		return;

	m_CommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
	m_NumBarriersToFlush = 0;
}

void CommandContext::TransitionResource(GpuResource & Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
	D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

	if (OldState != NewState)
	{
		K3D_ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Transition.pResource = Resource.GetResource();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = OldState;
		BarrierDesc.Transition.StateAfter = NewState;

		// Check to see if we already started the transition
		if (NewState == Resource.m_TransitioningState)
		{
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
		}
		else
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		Resource.m_UsageState = NewState;
	}
	else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) 
	{
		//InsertUAVBarrier(Resource, FlushImmediate);
	}
	if (FlushImmediate || m_NumBarriersToFlush == 16)
	{
		m_CommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
		m_NumBarriersToFlush = 0;
	}
}

NS_K3D_D3D12_END