#include "VkCommon.h"
#include "VkRHI.h"
#include "VkUtils.h"
#include "VkEnums.h"

#include <sstream>
#include <iomanip>

K3D_VK_BEGIN

using namespace Os;

CommandQueue::CommandQueue(Device::Ptr pDevice, VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex)
	: DeviceChild(pDevice)
{
	Initialize(queueTypes, queueFamilyIndex, queueIndex);
}

CommandQueue::~CommandQueue()
{
	Destroy();
}

void CommandQueue::Submit(
	const std::vector<VkCommandBuffer>& cmdBufs,
	const std::vector<VkSemaphore>& waitSemaphores, 
	const std::vector<VkPipelineStageFlags>& waitStageMasks,
	VkFence fence, const std::vector<VkSemaphore>& signalSemaphores)
{
	const VkPipelineStageFlags* pWaitDstStageMask = (waitSemaphores.size() == waitStageMasks.size()) ? waitStageMasks.data() : nullptr;
	VkSubmitInfo submits = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submits.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
	submits.pWaitSemaphores = waitSemaphores.empty() ? nullptr : waitSemaphores.data();
	submits.pWaitDstStageMask = pWaitDstStageMask;
	submits.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	submits.pCommandBuffers = cmdBufs.empty() ? nullptr : cmdBufs.data();
	submits.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submits.pSignalSemaphores = signalSemaphores.empty() ? nullptr : signalSemaphores.data();
	this->Submit({ submits }, fence);
}

VkResult CommandQueue::Submit(const std::vector<VkSubmitInfo>& submits, VkFence fence)
{
	VKRHI_METHOD_TRACE
	K3D_ASSERT(!submits.empty());
	uint32_t submitCount = static_cast<uint32_t>(submits.size());
	const VkSubmitInfo* pSubmits = submits.data();
	VkResult err = vkCmd::QueueSubmit(m_Queue, submitCount, pSubmits, fence);
	K3D_ASSERT((err == VK_SUCCESS) || (err == VK_ERROR_DEVICE_LOST));
	return err;
}

void CommandQueue::WaitIdle()
{
	K3D_VK_VERIFY(vkQueueWaitIdle(m_Queue));
}

void CommandQueue::Initialize(VkQueueFlags queueTypes, uint32 queueFamilyIndex, uint32 queueIndex)
{
	m_QueueFamilyIndex = queueFamilyIndex;
	m_QueueIndex = queueIndex;
	vkGetDeviceQueue(GetRawDevice(), m_QueueFamilyIndex, m_QueueIndex, &m_Queue);
}

void CommandQueue::Destroy()
{
	if (m_Queue == VK_NULL_HANDLE)
		return;
	m_Queue = VK_NULL_HANDLE;
}

CommandContextPool::CommandContextPool(Device::Ptr pDevice) 
	: DeviceChild(pDevice)
{
}

CommandContextPool::~CommandContextPool()
{
	VKLOG(Info, "CommandContextPool-Destroying....");
}

CommandContext* CommandContextPool::RequestContext(rhi::ECommandType type)
{
	::Os::Mutex::AutoLock lock(&m_ContextMutex);
	PtrCmdAlloc pAllocator = RequestCommandAllocator();
	auto info = CmdBufAllocInfo::Create(pAllocator->GetCommandPool());
	switch (type)
	{
	case rhi::ECMD_Bundle:
		info = CmdBufAllocInfo::Create(pAllocator->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	case rhi::ECMD_Graphics:
	case rhi::ECMD_Compute:
		break;
	}
	VkCommandBuffer cmdBuffer;
	K3D_VK_VERIFY(vkAllocateCommandBuffers(GetRawDevice(), &info, &cmdBuffer));
	auto context = new CommandContext(GetDevice(), cmdBuffer, pAllocator->GetCommandPool(), type);
	//uint32 tid = Thread::GetId();
	//m_ContextList[tid].push_back(context);
	VKLOG(Info, "CommandContextPool::RequestContext() called in thread [%s], (cmdBuf=0x%x, type=%d).", Thread::GetCurrentThreadName().c_str(), cmdBuffer, type);
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
		VKLOG(Info, "CommandContextPool, new vkcommandpool created. thread=%s", Thread::GetCurrentThreadName().c_str());
	}
	return m_AllocatorPool.at(tid);
}


CommandContext::CommandContext(Device::Ptr pDevice)
	: DeviceChild(pDevice)
{
}

CommandContext::CommandContext(Device::Ptr pDevice, VkCommandBuffer cmdBuf, VkCommandPool pool, rhi::ECommandType type)
	: DeviceChild(pDevice), m_CommandBuffer(cmdBuf), m_CommandPool(pool), m_CmdType(type)
{
}

CommandContext::~CommandContext()
{
	VKRHI_METHOD_TRACE
	vkFreeCommandBuffers(GetRawDevice(), m_CommandPool, 1, &m_CommandBuffer);
}

void CommandContext::Detach(rhi::IDevice * pDevice)
{

}

void CommandContext::CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src, rhi::CopyBufferRegion const& Region)
{
	vkCmd::CopyBuffer(m_CommandBuffer, (VkBuffer)Src.GetResourceLocation(), (VkBuffer)Dest.GetResourceLocation(), 1, (const VkBufferCopy*)&Region);
}

void CommandContext::CopyTexture(const rhi::TextureCopyLocation & Dest, const rhi::TextureCopyLocation & Src)
{
	K3D_ASSERT(Dest.pResource && Src.pResource);
	if (Src.pResource->GetResourceType() == rhi::EGT_Buffer && Dest.pResource->GetResourceType() != rhi::EGT_Buffer)
	{
		DynArray<VkBufferImageCopy> Copies;
		for (auto footprint : Src.SubResourceFootPrints)
		{
			VkBufferImageCopy bImgCpy = {};
			bImgCpy.bufferOffset = footprint.BufferOffSet;
			bImgCpy.imageOffset = { footprint.TOffSetX, footprint.TOffSetY, footprint.TOffSetZ };
			bImgCpy.imageExtent = { footprint.Footprint.Dimension.Width, footprint.Footprint.Dimension.Height, footprint.Footprint.Dimension.Depth };
			bImgCpy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			Copies.Append(bImgCpy);
		}
		vkCmdCopyBufferToImage(m_CommandBuffer, (VkBuffer)Src.pResource->GetResourceLocation(), (VkImage)Dest.pResource->GetResourceLocation(), 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Copies.Count(), Copies.Data());
	}
}

void CommandContext::Execute(bool Wait)
{
	VKRHI_METHOD_TRACE
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	if (m_CurrentRenderTarget)
	{
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
	} 
	else 
	{
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
	}
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffer;

	SpCmdQueue q;
	switch (m_CmdType)
	{
		case rhi::ECMD_Compute:
			q = GetComputeCmdQueue();
			break;
		case rhi::ECMD_Graphics:
		case rhi::ECMD_Bundle:
		default:
			q = GetImmCmdQueue();
			break;
	}
	if(q)
	{
		q->Submit({ submitInfo }, VK_NULL_HANDLE);
		if(Wait)
		{
			q->WaitIdle();
		}
	}
}

void CommandContext::Reset()
{
	vkResetCommandBuffer(m_CommandBuffer, 0);
}

void CommandContext::SubmitAndWait(PtrSemaphore wait, PtrSemaphore signal, PtrFence fence)
{
	VKRHI_METHOD_TRACE
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	VkPipelineStageFlags waitStage[1] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
	VkSemaphore waitSemaphore = wait ? wait->m_Semaphore : VK_NULL_HANDLE;
	VkSemaphore signalSemaphore = signal ? signal->m_Semaphore : VK_NULL_HANDLE;
	submitInfo.pWaitDstStageMask = waitStage;
	submitInfo.waitSemaphoreCount = wait ? 1:0;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffer;
	submitInfo.signalSemaphoreCount = signal? 1:0 ;
	submitInfo.pSignalSemaphores = &signalSemaphore;
#if _DEBUG
	std::stringstream strream;
	strream << "submit ----- wait " << std::hex << std::setfill('0') << waitSemaphore << ", signal " << std::hex
		<< std::setfill('0') << signalSemaphore;
	VKLOG(Info, strream.str().c_str());
#endif
	SpCmdQueue q;
	switch (m_CmdType)
	{
		case rhi::ECMD_Compute:
			q = GetComputeCmdQueue();
			break;
		case rhi::ECMD_Graphics:
		case rhi::ECMD_Bundle:
		default:
			q = GetImmCmdQueue();
			break;
	}
	q->Submit({ submitInfo }, fence ? fence->m_Fence : VK_NULL_HANDLE);
}

void CommandContext::InitCommandBufferPool()
{
}

void CommandContext::Begin()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;
	K3D_VK_VERIFY(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
}

void CommandContext::BeginRendering() 
{
	if (!m_IsRenderPassActive)
	{
		
	}
}

void CommandContext::End()
{
	if (m_IsRenderPassActive)
	{
		EndRenderPass();
		m_IsRenderPassActive = false;
	}
	vkEndCommandBuffer(m_CommandBuffer);
}

void CommandContext::EndRendering()
{
	if (m_IsRenderPassActive)
	{
		vkCmdEndRenderPass(m_CommandBuffer);
		m_IsRenderPassActive = false;
	}
}

void CommandContext::PresentInViewport(rhi::RenderViewportRef rvp)
{
	VKRHI_METHOD_TRACE
	RenderViewport* vp = static_cast<RenderViewport*>(rvp.Get());
	//vp->PrepareNextFrame();
	SubmitAndWait(vp->GetPresentSemaphore(), vp->GetRenderSemaphore(), nullptr);
	vp->Present(false);
}

void CommandContext::BindDescriptorSets(
	VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
	uint32 firstSet, uint32 descriptorSetCount, const VkDescriptorSet * pDescriptorSets, 
	uint32 dynamicOffsetCount, const uint32 * pDynamicOffsets)
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

void CommandContext::ClearColorImage(SpTexture colorBuffer, const VkClearColorValue * pColor, VkImageLayout imageLayout)
{
	VkImage clearImage = colorBuffer->Get();
	VkImageViewCreateInfo clearImageView = colorBuffer->GetViewInfo();
	ClearColorImage(clearImage, imageLayout, pColor, 1, &clearImageView.subresourceRange);
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
	vkCmdPipelineBarrier(m_CommandBuffer,
		srcStageMask, dstStageMask, dependencyFlags, 
		memoryBarrierCount, pMemoryBarriers,
		bufferMemoryBarrierCount, pBufferMemoryBarriers, 
		imageMemoryBarrierCount, pImageMemoryBarriers);
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
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
	{
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	}
	case VK_IMAGE_LAYOUT_UNDEFINED:
	{
		barrier.srcAccessMask = 0;
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
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
	{
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
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
		break;
	}
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
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

void CommandContext::ClearColorBuffer(rhi::GpuResourceRef gpuRes, kMath::Vec4f const& color)
{
	VkImageSubresourceRange image_subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	vkCmdClearColorImage(m_CommandBuffer, (VkImage)gpuRes->GetResourceLocation(),
		g_ResourceState[ gpuRes->GetUsageState() ], (const VkClearColorValue*)&color, 1, &image_subresource_range);
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
	r2d.extent.height = rect.Bottom - rect.Top;
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
	VkBuffer buf = (VkBuffer)(IBView.BufferLocation);
	vkCmdBindIndexBuffer(m_CommandBuffer, buf, 0, VK_INDEX_TYPE_UINT32);
}

void CommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView)
{
	VkBuffer buf = (VkBuffer)(VBView.BufferLocation);
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(m_CommandBuffer, Slot, 1, &buf, offsets);
}

void CommandContext::SetPipelineState(uint32 hashCode, rhi::PipelineStateObjectRef pipelineState)
{
	K3D_ASSERT(pipelineState);
	PipelineStateObject * gfxPso = static_cast<PipelineStateObject*>(pipelineState.Get());
	vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPso->m_Pipeline);
}

void CommandContext::SetPipelineLayout(rhi::PipelineLayoutRef pRHIPipelineLayout)
{
	K3D_ASSERT(pRHIPipelineLayout);
	auto pipelineLayout = StaticPointerCast<PipelineLayout>(pRHIPipelineLayout);
	VkDescriptorSet sets[] = { pipelineLayout->GetNativeDescriptorSet() };
	vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetNativeLayout(), 0, 1, sets, 0, NULL);
}

void CommandContext::SetPrimitiveType(rhi::EPrimitiveType)
{
	//
}

void CommandContext::DrawInstanced(rhi::DrawInstancedParam drawParam)
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

void CommandContext::SetRenderTarget(rhi::RenderTargetRef rt)
{
	if (!m_IsRenderPassActive)
	{
		RenderTarget* pRT = static_cast<RenderTarget*>(rt.Get());
		m_CurrentRenderTarget = pRT;
		// render pass attachments num should match that of framebuffer attachment
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = NULL;
		renderPassBeginInfo.renderPass = pRT->GetRenderpass();
		renderPassBeginInfo.renderArea = pRT->GetRenderArea();
		renderPassBeginInfo.pClearValues = pRT->m_ClearValues;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.framebuffer = pRT->GetFramebuffer();
		BeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_IsRenderPassActive = true;
	}
}

void CommandContext::Dispatch(uint32 x, uint32 y, uint32 z)
{
	vkCmdDispatch(m_CommandBuffer, x, y, z);
}

void CommandContext::TransitionResourceBarrier(rhi::GpuResourceRef resource,/* rhi::EPipelineStage stage,*/ rhi::EResourceState dstState)
{
	VKRHI_METHOD_TRACE
	auto pTex = k3d::DynamicPointerCast<Texture>(resource);
	if (pTex)
	{
		VkImageLayout srcLayout = g_ResourceState[resource->GetUsageState()];
		VkImageLayout dstLayout = g_ResourceState[dstState];
		ImageMemoryBarrierParams param(pTex->Get(), srcLayout, dstLayout);
		VkImageSubresourceRange texSubRange = pTex->GetSubResourceRange();
		param.SrcStageMask(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT).DstStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT).SubResourceRange(texSubRange);
		if (dstState == rhi::ERS_TransferDst || resource->GetUsageState() == rhi::ERS_TransferDst)
		{
			param.SrcStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT).DstStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		}
		PipelineBarrierImageMemory(param);
		pTex->m_UsageState = dstState;
		pTex->m_ImageLayout = dstLayout;
	}
}

void CommandContext::ExecuteBundle(rhi::ICommandContext * pCmd)
{
	auto pCmdCtx = static_cast<CommandContext*>(pCmd);
	VkCommandBuffer buffer[] = { pCmdCtx->m_CommandBuffer };
	vkCmdExecuteCommands(m_CommandBuffer, 1, buffer);
}

K3D_VK_END