#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Base/vulkantools.h"

K3D_VK_BEGIN

CommandContext::~CommandContext()
{
	Log::Out(LogLevel::Info, "CommandContext", "Destroying....");
	vkDestroyCommandPool(GetRawDevice(), m_CommandPool, nullptr);
}

void CommandContext::Detach(rhi::IDevice * pDevice)
{

}

void CommandContext::CopyBuffer(rhi::IGpuResource& Dest, rhi::IGpuResource& Src)
{

}

void CommandContext::Execute(bool Wait)
{

}

void CommandContext::Reset()
{

}

void CommandContext::InitCommandBufferPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = 0;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	K3D_VK_VERIFY(vkCreateCommandPool(GetRawDevice(), &cmdPoolInfo, nullptr, &m_CommandPool));
}

void GraphicsCommandContext::InitCommandBufferAndPass()
{
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vkTools::initializers::commandBufferAllocateInfo(
			m_CommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);
	K3D_VK_VERIFY(vkAllocateCommandBuffers(GetRawDevice(), &cmdBufAllocateInfo, &m_CommandBuffer));
}

void GraphicsCommandContext::ClearColorBuffer(rhi::IColorBuffer* iColorBuffer)
{

}

void GraphicsCommandContext::ClearDepthBuffer(rhi::IDepthBuffer* iDepthBuffer)
{

}

void GraphicsCommandContext::SetRenderTargets(uint32 NumColorBuffer, rhi::IColorBuffer *, rhi::IDepthBuffer *, bool ReadOnlyDepth)
{

}

void GraphicsCommandContext::SetViewport(const rhi::ViewportDesc & viewport)
{
	//vkCmdSetViewport(m_CommandBuffer, 1, &viewport);
}

void GraphicsCommandContext::SetScissorRects(uint32, const rhi::Rect*)
{

}

void GraphicsCommandContext::SetIndexBuffer(const rhi::IndexBufferView& IBView)
{

}

void GraphicsCommandContext::SetVertexBuffer(uint32 Slot, const rhi::VertexBufferView& VBView)
{

}

void GraphicsCommandContext::SetPipelineState(uint32 hashCode, rhi::IPipelineStateObject*)
{

}

void GraphicsCommandContext::SetPipelineLayout(rhi::IPipelineLayout * pRHIPipelineLayout)
{

}

void GraphicsCommandContext::SetPrimitiveType(rhi::EPrimitiveType)
{

}

void GraphicsCommandContext::DrawInstanced(rhi::DrawInstanceParam)
{

}

void GraphicsCommandContext::DrawIndexedInstanced(rhi::DrawIndexedInstancedParam)
{

}

void ComputeCommandContext::Dispatch(uint32 x, uint32 y, uint32 z)
{

}

void ComputeCommandContext::SetPipelineLayout(rhi::IPipelineLayout *)
{

}

K3D_VK_END