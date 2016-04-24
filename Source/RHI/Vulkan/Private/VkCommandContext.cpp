#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Base/vulkantools.h"
#include "VkUtils.h"

K3D_VK_BEGIN

using namespace Concurrency;

CommandContextPool::CommandContextPool(Device::Ptr pDevice) 
	: DeviceChild(pDevice)
{}

CommandContextPool::~CommandContextPool()
{
	Log::Out(LogLevel::Info, "CommandContextPool", "Destroying....");
}

CommandContext* CommandContextPool::RequestContext()
{
	m_ContextMutex.Lock();
	PtrCmdAlloc pAllocator = RequestCommandAllocator();
	auto info = CmdBufAllocInfo::Create(pAllocator->GetCommandPool());
	VkCommandBuffer cmdBuffer;
	K3D_VK_VERIFY(vkAllocateCommandBuffers(GetRawDevice(), &info, &cmdBuffer));
	auto context = new CommandContext(GetDevice(), cmdBuffer);
	uint32 tid = Thread::GetId();
	m_ContextList[tid].push_back(context);
	Log::Out(LogLevel::Info, "CommandContextPool", "thread %d, new GraphicsCommandContext created.", tid);
	m_ContextMutex.UnLock();
	return context;
}

PtrCmdAlloc CommandContextPool::RequestCommandAllocator()
{
	uint32 tid = Thread::GetId();
	if (m_AllocatorPool.find(tid) == m_AllocatorPool.end())
	{
		Mutex::AutoLock lock(&m_PoolMutex);
		auto pAllocator = GetDevice()->NewCommandAllocator(false);
		m_AllocatorPool.insert({ tid, pAllocator });
		Log::Out(LogLevel::Info, "CommandContextPool", "thread %d, new vkcommandpool created.", tid);
	}
	return m_AllocatorPool.at(tid);
}


CommandContext::CommandContext(Device::Ptr pDevice)
	: DeviceChild(pDevice)
{
}

CommandContext::CommandContext(Device::Ptr pDevice, VkCommandBuffer cmdBuf)
	: DeviceChild(pDevice), m_CommandBuffer(cmdBuf) 
{
}

CommandContext::~CommandContext()
{
	Log::Out(LogLevel::Info, "CommandContext", "Destroying....");
}

void CommandContext::Detach(rhi::IDevice * pDevice)
{

}

void CommandContext::CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src)
{
//	vkCmdCopyBuffer(m_CommandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void CommandContext::Execute(bool Wait)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffer;
	K3D_VK_VERIFY(vkQueueSubmit(GetRawQueue(), 1, &submitInfo, VK_NULL_HANDLE));
}

void CommandContext::Reset()
{

}

void CommandContext::SubmitAndWait(PtrSemaphore wait, PtrSemaphore signal, PtrFence fence)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait? &wait->m_Semaphore : VK_NULL_HANDLE;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal? &signal->m_Semaphore : VK_NULL_HANDLE;
	K3D_VK_VERIFY(vkQueueSubmit(GetRawQueue(), 1, &submitInfo, fence? fence->m_Fence: VK_NULL_HANDLE));
}

void CommandContext::InitCommandBufferPool()
{
}

void CommandContext::Begin()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	K3D_VK_VERIFY(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
}

void CommandContext::End()
{
	vkEndCommandBuffer(m_CommandBuffer);
}

void CommandContext::BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32 firstSet, uint32 descriptorSetCount, const VkDescriptorSet * pDescriptorSets, uint32 dynamicOffsetCount, const uint32 * pDynamicOffsets)
{
	vkCmdBindDescriptorSets(m_CommandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void CommandContext::BindDescriptorSet(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, const VkDescriptorSet & pDescriptorSets)
{
	const VkDescriptorSet descSets[1] = { pDescriptorSets };
	BindDescriptorSets(pipelineBindPoint, layout, 0, 1, descSets, 0, nullptr);
}

void CommandContext::ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue * pColor, uint32 rangeCount, const VkImageSubresourceRange * pRanges)
{
	vkCmdClearColorImage(m_CommandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void CommandContext::ClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue * pDepthStencil, uint32 rangeCount, const VkImageSubresourceRange * pRanges)
{
	vkCmdClearDepthStencilImage(m_CommandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void CommandContext::ClearAttachments(uint32 attachmentCount, const VkClearAttachment * pAttachments, uint32 rectCount, const VkClearRect * pRects)
{
	vkCmdClearAttachments(m_CommandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void CommandContext::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32 memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers, uint32 bufferMemoryBarrierCount, const VkBufferMemoryBarrier * pBufferMemoryBarriers, uint32 imageMemoryBarrierCount, const VkImageMemoryBarrier * pImageMemoryBarriers)
{
	vkCmdPipelineBarrier(m_CommandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CommandContext::PipelineBarrierBufferMemory(const BufferMemoryBarrierParams & params)
{
	VkBufferMemoryBarrier barrier = params.m_Barrier;
	PipelineBarrier(params.m_SrcStageMask, params.m_DstStageMask, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

void CommandContext::PipelineBarrierImageMemory(const ImageMemoryBarrierParams & params)
{
	VkImageMemoryBarrier barrier = params.m_Barrier;
	switch (params.m_Barrier.oldLayout)
	{
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
	{
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
	{
		barrier.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	}
	}
	switch (params.m_Barrier.newLayout)
	{
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
	{
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	}

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
	{
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
	{
		barrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	}
	}
	PipelineBarrier(params.m_SrcStageMask, params.m_DstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void CommandContext::PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32 offset, uint32 size, const void * pValues)
{
	vkCmdPushConstants(m_CommandBuffer, layout, stageFlags, offset, size, pValues);
}

void CommandContext::BeginRenderPass(const VkRenderPassBeginInfo * pRenderPassBegin, VkSubpassContents contents)
{
	vkCmdBeginRenderPass(m_CommandBuffer, pRenderPassBegin, contents);
}

void CommandContext::SetScissorRects(uint32 count, VkRect2D * pRects)
{
	vkCmdSetScissor(m_CommandBuffer, 0, count, pRects);
}

void CommandContext::ClearColorBuffer(rhi::IColorBuffer* iColorBuffer)
{

}

void CommandContext::ClearDepthBuffer(rhi::IDepthBuffer* iDepthBuffer)
{

}

void CommandContext::SetRenderTargets(uint32 NumColorBuffer, rhi::IColorBuffer *, rhi::IDepthBuffer *, bool ReadOnlyDepth)
{

}

void CommandContext::SetViewport(const rhi::ViewportDesc & viewport)
{
	vkCmdSetViewport(m_CommandBuffer, 0, 1, reinterpret_cast<const VkViewport*>(&viewport));
}

/**
 * @see #RHIRect2VkRect2D
 */
void RHIRect2VkRect(const rhi::Rect & rect, VkRect2D & r2d)
{
	r2d.extent.width = rect.Right - rect.Left;
	r2d.extent.width = rect.Bottom - rect.Top;
	r2d.offset.x = rect.Left;
	r2d.offset.y = rect.Top;
}

void CommandContext::SetScissorRects(uint32 count, const rhi::Rect* pRect)
{
	std::vector<VkRect2D> scissorRects(count);
	for (uint32 i = 0; i < count; i++) 
	{
		RHIRect2VkRect(pRect[i], scissorRects[i]);
	}
	vkCmdSetScissor(m_CommandBuffer, 0, count, scissorRects.data());
}

void CommandContext::SetIndexBuffer(const rhi::IndexBufferView& IBView)
{
	VkBuffer buf = (VkBuffer)IBView.BufferLocation;
	vkCmdBindIndexBuffer(m_CommandBuffer, buf, 0, VK_INDEX_TYPE_UINT32);
}

void CommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView)
{
	VkBuffer buf = (VkBuffer)VBView.BufferLocation;
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(m_CommandBuffer, Slot, 1, &buf, offsets);
}

void CommandContext::SetPipelineState(uint32 hashCode, rhi::IPipelineStateObject* pipelineState)
{
	K3D_ASSERT(pipelineState!=nullptr);
	PipelineStateObject * gfxPso = static_cast<PipelineStateObject*>(pipelineState);
	vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPso->m_Pipeline);
}

void CommandContext::SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout)
{
	K3D_ASSERT(pRHIPipelineLayout!=nullptr);
//	PipelineLayout * pipelineLayout = static_cast<PipelineLayout*>(pRHIPipelineLayout);
	//vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->, 0, 1, &descriptorSet, 0, NULL);
}

void CommandContext::SetPrimitiveType(rhi::EPrimitiveType)
{
	//
}

void CommandContext::DrawInstanced(rhi::DrawInstanceParam drawParam)
{
	vkCmdDraw(m_CommandBuffer, drawParam.VertexCountPerInstance, drawParam.InstanceCount,
		drawParam.StartVertexLocation, drawParam.StartInstanceLocation);
}

void CommandContext::DrawIndexedInstanced(rhi::DrawIndexedInstancedParam drawParam)
{
	vkCmdDrawIndexed(m_CommandBuffer, drawParam.IndexCountPerInstance, drawParam.InstanceCount,
		drawParam.StartIndexLocation, drawParam.BaseVertexLocation, drawParam.StartInstanceLocation);
}

void CommandContext::NextSubpass(VkSubpassContents contents)
{
	vkCmdNextSubpass(m_CommandBuffer, contents);
}

void CommandContext::EndRenderPass()
{
	vkCmdEndRenderPass(m_CommandBuffer);
}

void CommandContext::Dispatch(uint32 x, uint32 y, uint32 z)
{
	vkCmdDispatch(m_CommandBuffer, x, y, z);
}

K3D_VK_END