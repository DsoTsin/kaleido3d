#include "VkCommon.h"
#include "VkObjects.h"
#include "VkEnums.h"

K3D_VK_BEGIN

RenderTargetLayout::RenderTargetLayout(rhi::RenderTargetLayout const &layout)
{

}

RenderpassAttachment::RenderpassAttachment(VkFormat fmt, VkSampleCountFlagBits samples)
{
	m_Description.LoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
		.StoreOp(VK_ATTACHMENT_STORE_OP_STORE)
		.StencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
		.StencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
		.InitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		.FinalLayout(VK_IMAGE_LAYOUT_UNDEFINED)
		.Format(fmt)
		.Samples(samples);

	m_ClearValue.color.float32[0] = 0.0f;
	m_ClearValue.color.float32[1] = 0.0f;
	m_ClearValue.color.float32[2] = 0.0f;
	m_ClearValue.color.float32[3] = 0.0f;
	m_ClearValue.depthStencil.depth = 0.0f;
	m_ClearValue.depthStencil.stencil = 0;
}

RenderpassAttachment RenderpassAttachment::CreateColor(VkFormat format, VkSampleCountFlagBits samples)
{
	RenderpassAttachment result(format);
	result.GetDescription().Samples(samples).FinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	return result;
}

RenderpassAttachment RenderpassAttachment::CreateDepthStencil(VkFormat format, VkSampleCountFlagBits samples)
{
	RenderpassAttachment result(format);
	result.GetDescription().Samples(samples).FinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	return result;
}

Subpass & Subpass::AddColorAttachment(uint32 attachmentIndex, uint32 resolveAttachmentIndex)
{
	m_ColorAttachments.push_back(attachmentIndex); m_ResolveAttachments.push_back(resolveAttachmentIndex);
	return *this;
}

Subpass & Subpass::AddDepthStencilAttachment(uint32 attachmentIndex)
{
	m_DepthStencilAttachment.push_back(attachmentIndex);
	return *this;
}

Subpass & Subpass::AddPreserveAttachment(uint32 attachmentIndex)
{
	m_PreserveAttachments.push_back(attachmentIndex);
	return *this;
}

Subpass & Subpass::AddPreserveAttachments(const std::vector<uint32>& attachmentIndices)
{
	std::copy(std::begin(attachmentIndices), std::end(attachmentIndices), std::back_inserter(m_PreserveAttachments));
	return *this;
}

SubpassDependency::SubpassDependency(uint32 srcSubpass, uint32 dstSubpass)
{
	m_Dependency = {};
	m_Dependency.srcSubpass = srcSubpass;
	m_Dependency.dstSubpass = dstSubpass;
}

SubpassDependency & SubpassDependency::SrcStageMask(VkPipelineStageFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.srcStageMask = mask;
	}
	else {
		m_Dependency.srcStageMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::DstStageMask(VkPipelineStageFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.dstStageMask = mask;
	}
	else {
		m_Dependency.dstStageMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::StageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive)
{
	return SrcStageMask(srcStageMask, exclusive).DstStageMask(dstStageMask, exclusive);
}

SubpassDependency & SubpassDependency::SrcAccessMask(VkAccessFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.srcAccessMask = mask;
	}
	else {
		m_Dependency.srcAccessMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::DstAccessMask(VkAccessFlags mask, bool exclusive)
{
	if (exclusive) {
		m_Dependency.dstAccessMask = mask;
	}
	else {
		m_Dependency.dstAccessMask |= mask;
	}
	return *this;
}

SubpassDependency & SubpassDependency::AccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive)
{
	return SrcAccessMask(srcAccessMask).DstAccessMask(dstAccessMask);
}

RenderpassOptions::RenderpassOptions(VkFormat colorFormat, VkSampleCountFlagBits samples)
{
	AddAttachment(RenderpassAttachment::CreateColor(colorFormat, samples));
	Subpass& subPass = Subpass().AddColorAttachment(0);
	AddSubPass(subPass);
}

RenderpassOptions::RenderpassOptions(VkFormat colorFormat, VkFormat depthStencilFormat, VkSampleCountFlagBits samples)
{
	AddAttachment(RenderpassAttachment::CreateColor(colorFormat, samples));
	AddAttachment(RenderpassAttachment::CreateDepthStencil(depthStencilFormat, samples));
	Subpass subPass = Subpass().AddColorAttachment(0).AddDepthStencilAttachment(1);
	AddSubPass(subPass);
}

BufferMemoryBarrierParams::BufferMemoryBarrierParams()
{
	m_Barrier = {};
	m_Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	m_Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	m_Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

BufferMemoryBarrierParams::BufferMemoryBarrierParams(VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	: BufferMemoryBarrierParams()
{
	m_Barrier.buffer = buffer;
	m_Barrier.srcAccessMask = srcAccessMask;
	m_Barrier.dstAccessMask = srcAccessMask;

	m_SrcStageMask = srcStageMask;
	m_DstStageMask = dstStageMask;
}

ImageMemoryBarrierParams::ImageMemoryBarrierParams()
{
	m_Barrier = {};
	m_Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	m_Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	m_Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

ImageMemoryBarrierParams::ImageMemoryBarrierParams(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	: ImageMemoryBarrierParams()
{
	m_Barrier.image = image;
	m_Barrier.oldLayout = oldLayout;
	m_Barrier.newLayout = newLayout;

	m_SrcStageMask = srcStageMask;
	m_DstStageMask = dstStageMask;
}

ImageInfo ImageInfo::FromRHI(rhi::TextureDesc const & desc)
{
	ImageInfo info;
	info.Format(g_FormatTable[desc.Format])
		.MipLevel(desc.MipLevels)
		.Layers(desc.Layers)
		.Dimens(desc.Width, desc.Height, desc.Depth);
	return info;
}


std::pair<VkImageView, VkImageViewCreateInfo> ImageViewInfo::CreateColorImageView(VkDevice device, VkFormat colorFmt, VkImage colorImage, VkImageAspectFlags aspectMask)
{
	VkImageView imageView = VK_NULL_HANDLE;
	VkImageViewCreateInfo info = CreateColorImageInfo(colorFmt, colorImage);
	K3D_ASSERT(0 != info.subresourceRange.aspectMask);
	if (VK_IMAGE_ASPECT_COLOR_BIT == info.subresourceRange.aspectMask)
	{
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
	}
	K3D_VK_VERIFY(vkCreateImageView(device, &info, nullptr, &imageView));
	return std::make_pair(std::move(imageView), std::move(info));
}

std::pair<VkImageView, VkImageViewCreateInfo> ImageViewInfo::CreateDepthStencilImageView(VkDevice device, VkFormat colorFmt, VkImage colorImage, VkImageAspectFlags aspectMask)
{
	VkImageView imageView = VK_NULL_HANDLE;
	VkImageViewCreateInfo info = CreateDepthStencilImageInfo(colorFmt, colorImage);
	K3D_ASSERT(0 != info.subresourceRange.aspectMask);
	K3D_VK_VERIFY(vkCreateImageView(device, &info, nullptr, &imageView));
	return  std::make_pair(std::move(imageView), std::move(info));
}

ImageViewInfo ImageViewInfo::From(ImageInfo const & info, VkImage image)
{
	ImageViewInfo imageViewInfo;
	imageViewInfo.Image(image).ViewType(info.GuessViewType())
		.Format(info.format);
	imageViewInfo.flags = 0;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = info.arrayLayers;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = info.mipLevels;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	return imageViewInfo;
}

K3D_VK_END
