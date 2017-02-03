#include "VkCommon.h"
#include "VkRHI.h"
#include "VkUtils.h"

K3D_VK_BEGIN

FrameBuffer::FrameBuffer(Device::Ptr pDevice, VkRenderPass renderPass, FrameBuffer::Option const& op)
	: DeviceChild(pDevice)
	, m_RenderPass(renderPass)
	, m_Width(op.Width)
	, m_Height(op.Height)
{
	VKRHI_METHOD_TRACE
	for (auto& elem : op.Attachments)
	{
		if (elem.ImageAttachment)
		{
			continue;
		}
	}

	std::vector<VkImageView> attachments;
	for (const auto& elem : op.Attachments) {
		attachments.push_back(elem.ImageAttachment);
	}

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.renderPass = m_RenderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(op.Attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.width = m_Width;
	createInfo.height = m_Height;
	createInfo.layers = 1;
	createInfo.flags = 0;
	K3D_VK_VERIFY(GetGpuRef()->vkCreateFramebuffer(GetRawDevice(), &createInfo, nullptr, &m_FrameBuffer));
}

FrameBuffer::FrameBuffer(Device::Ptr pDevice, RenderPass * renderPass, RenderTargetLayout const &)
	: DeviceChild(pDevice)
{
}

FrameBuffer::~FrameBuffer()
{
	if (VK_NULL_HANDLE == m_FrameBuffer)
	{
		return;
	}
	VKLOG(Info, "FrameBuffer destroy... -- %0x.", m_FrameBuffer);
	GetGpuRef()->vkDestroyFramebuffer(GetRawDevice(), m_FrameBuffer, nullptr);
	m_FrameBuffer = VK_NULL_HANDLE;
}

FrameBuffer::Attachment::Attachment(VkFormat format, VkSampleCountFlagBits samples)
{
	VkImageAspectFlags aspectMask = DetermineAspectMask(Format);
	if (VK_IMAGE_ASPECT_COLOR_BIT == aspectMask)
	{
		FormatFeatures = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
	}
	else
	{
		FormatFeatures = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
}

K3D_VK_END