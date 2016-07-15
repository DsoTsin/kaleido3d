#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "Private/D3D12Enums.h"

NS_K3D_D3D12_BEGIN

const char * TAG_COMMAND_CONTEXT = "CommandContext";
const char * TAG_COMPUTE_CONTEXT = "ComputeContext";
const char * TAG_GRAPHICS_CONTEXT = "GraphicsContext";

CommandContext::CommandContext(Device::Ptr pDevice) :
	//m_DynamicDescriptorHeap(*this),
	m_CpuLinearAllocator(kCpuWritable, pDevice),
	m_GpuLinearAllocator(kGpuExclusive, pDevice)
{
	m_OwningManager = nullptr;
	m_CommandList = nullptr;
	m_CurrentAllocator = nullptr;
	ZeroMemory(m_CurrentDescriptorHeaps, sizeof(m_CurrentDescriptorHeaps));
	m_NumBarriersToFlush = 0;
#if _DEBUG
	Log::Out(LogLevel::Info, TAG_COMMAND_CONTEXT, "Allocated.");
#endif
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

void CommandContext::CopyBuffer(rhi::IGpuResource & Dest, rhi::IGpuResource & Src, ::k3d::DynArray<rhi::BufferRegion> const& Regions)
{
}

void CommandContext::SetRenderTarget(rhi::IRenderTarget *)
{
}

void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr)
{

}

void CommandContext::Reset()
{
	K3D_ASSERT(m_CommandList != nullptr && m_CurrentAllocator == nullptr);
	m_CurrentAllocator = m_OwningManager->RequestAllocator();
	m_CommandList->Reset(m_CurrentAllocator, nullptr);
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
	//m_DynamicDescriptorHeap.CleanupUsedHeaps(FenceValue);

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

void CommandContext::ClearColorBuffer(rhi::IGpuResource*, kMath::Vec4f const & )
{
}

void CommandContext::ClearDepthBuffer(rhi::IDepthBuffer * iDepthBuffer)
{
}

void CommandContext::SetRenderTargets(
	uint32 NumColorBuffer, rhi::IColorBuffer * ColorBuffers,
	rhi::IDepthBuffer * iDepthBuffer, bool ReadOnlyDepth)
{
	
	//m_CommandList->OMSetRenderTargets(NumColorBuffer, )
}

void CommandContext::SetIndexBuffer(const rhi::IndexBufferView & IBView)
{
	m_CommandList->IASetIndexBuffer((const D3D12_INDEX_BUFFER_VIEW*)&IBView);
}

void CommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView)
{
	m_CommandList->IASetVertexBuffers(Slot, 1, (const D3D12_VERTEX_BUFFER_VIEW*)&VBView);
}

void CommandContext::SetPipelineState(uint32 Hash, rhi::IPipelineStateObject *RhiPipeLineState)
{
	PipelineState * pPSO = static_cast<PipelineState*>(RhiPipeLineState);
	K3D_ASSERT(pPSO != nullptr);
	m_CommandList->SetPipelineState(pPSO->GetPipelineStateObject());
}

void CommandContext::SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout)
{
	PipelineLayout * pipelineLayout = static_cast<PipelineLayout*>(pRHIPipelineLayout);
	m_CommandList->SetGraphicsRootSignature(pipelineLayout->GetRootSignature());
}

void CommandContext::SetViewport(const rhi::ViewportDesc& Vp)
{
	m_CommandList->RSSetViewports(1, (const D3D12_VIEWPORT*)&Vp);
}

void CommandContext::SetScissorRects(uint32 Num, const rhi::Rect *pRects)
{
	m_CommandList->RSSetScissorRects(Num, (const D3D12_RECT*)pRects);
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

void CommandContext::Dispatch(uint32 X, uint32 Y, uint32 Z)
{
	m_CommandList->Dispatch(X, Y, Z);
}

void CommandContext::TransitionResourceBarrier(rhi::IGpuResource * resource, rhi::EResourceState srcState, rhi::EResourceState dstState)
{
}

void CommandContext::SetRootSignature(const RootSignature &RootSig)
{
	if (RootSig.GetSignature() == m_CurGraphicsRootSignature)
		return;

	m_CommandList->SetGraphicsRootSignature(m_CurGraphicsRootSignature = RootSig.GetSignature());

	//m_DynamicDescriptorHeap.ParseGraphicsRootSignature(RootSig);
}

NS_K3D_D3D12_END