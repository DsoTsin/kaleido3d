#include "DXCommon.h"
#include "Public/D3D12RHI.h"
#include "Private/D3D12Enums.h"

NS_K3D_D3D12_BEGIN

CommandContext::CommandContext() :
	m_DynamicDescriptorHeap(*this),
	m_CpuLinearAllocator(kCpuWritable),
	m_GpuLinearAllocator(kGpuExclusive)
{

}

CommandContext::~CommandContext()
{

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

}

void CommandContext::Execute(bool Wait)
{

}

void CommandContext::FlushResourceBarriers()
{

}

NS_K3D_D3D12_END