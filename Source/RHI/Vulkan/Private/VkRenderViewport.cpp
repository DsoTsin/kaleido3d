#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"
#include "VkUtils.h"

#include <sstream>
#include <iomanip>

K3D_VK_BEGIN

RenderViewport::RenderViewport(
	rhi::IDevice * pDevice, 
	void * windowHandle, 
	rhi::GfxSetting & setting)
	: m_pSwapChain(new SwapChain(static_cast<Device::Ptr>(pDevice)))
	, m_NumBufferCount(-1)
	, DeviceChild(static_cast<Device::Ptr>(pDevice))
	, m_CurFrameId(0)
{
	m_pSwapChain->Initialize(windowHandle, setting);
	m_NumBufferCount = m_pSwapChain->GetBackBufferCount();
	VKLOG(Info, "RenderViewport-Initialized: width(%d), height(%d), swapImage num(%d).",
		  setting.Width, setting.Height, m_NumBufferCount);
	m_PresentSemaphore = GetDevice()->NewSemaphore();
	m_RenderSemaphore = GetDevice()->NewSemaphore();
	RHIRoot::AddViewport(this);
}

RenderViewport::~RenderViewport()
{
	VKLOG(Info, "RenderViewport-Destroyed..");
}

bool RenderViewport::InitViewport(void *windowHandle, rhi::IDevice * pDevice, rhi::GfxSetting & gfxSetting)
{
	AllocateDefaultRenderPass(gfxSetting);
	AllocateRenderTargets(gfxSetting);
	return true;
}

void RenderViewport::PrepareNextFrame()
{
	VKRHI_METHOD_TRACE
	m_CurFrameId = m_pSwapChain->AcquireNextImage(m_PresentSemaphore, nullptr);
	std::stringstream stream;
	stream << "Current Frame Id = " << m_CurFrameId << " acquiring " << std::hex << std::setfill('0') << m_PresentSemaphore->GetNativeHandle();
	VKLOG(Info, stream.str().c_str());
}

bool RenderViewport::Present(bool vSync)
{
	VKRHI_METHOD_TRACE
	std::stringstream stream;
	stream << "present ----- renderSemaphore " << std::hex << std::setfill('0') << m_RenderSemaphore->GetNativeHandle();
	VKLOG(Info, stream.str().c_str());
	VkResult result = m_pSwapChain->Present(m_CurFrameId, m_RenderSemaphore);
	return result==VK_SUCCESS;
}

rhi::IRenderTarget * RenderViewport::GetRenderTarget(uint32 index)
{
	return m_RenderTargets[index].get();
}

uint32 RenderViewport::GetSwapChainIndex()
{
	return m_CurFrameId;
}

uint32 RenderViewport::GetSwapChainCount()
{
	return m_pSwapChain->GetBackBufferCount();
}

void RenderViewport::AllocateDefaultRenderPass(rhi::GfxSetting & gfxSetting)
{
	VKRHI_METHOD_TRACE
	VkFormat colorformat = m_pSwapChain ? m_pSwapChain->GetFormat() : g_FormatTable[gfxSetting.ColorFormat];
	RenderpassAttachment colorAttach = RenderpassAttachment::CreateColor(colorformat);
	colorAttach.GetDescription()
		.InitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.FinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	RenderpassOptions option;
	option.AddAttachment(colorAttach);
	Subpass subpass_;
	subpass_.AddColorAttachment(0);
	if (gfxSetting.HasDepth) 
	{
		VkFormat depthFormat = g_FormatTable[gfxSetting.DepthStencilFormat];
		GetSupportedDepthFormat(GetPhysicalDevice(), &depthFormat);
		RenderpassAttachment depthAttach = 
			RenderpassAttachment::CreateDepthStencil(depthFormat);
		depthAttach.GetDescription().InitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.FinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		subpass_.AddDepthStencilAttachment(1);
		option.AddAttachment(depthAttach);
	}
	option.AddSubPass(subpass_);
	m_RenderPass = std::make_unique<RenderPass>(GetDevice(), option);
}

void RenderViewport::AllocateRenderTargets(rhi::GfxSetting & gfxSetting)
{
	if (m_RenderPass)
	{
		VkFormat depthFormat = g_FormatTable[gfxSetting.DepthStencilFormat];
		VkImage depthImage;
		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = depthFormat;
		// Use example's height and width
		image.extent = { GetWidth(), GetHeight(), 1 };
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vkCreateImage(GetRawDevice(), &image, nullptr, &depthImage);


		VkDeviceMemory depthMem;
		// Allocate memory for the image (device local) and bind it to our image
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(GetRawDevice(), depthImage, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex);
		vkAllocateMemory(GetRawDevice(), &memAlloc, nullptr, &depthMem);
		vkBindImageMemory(GetRawDevice(), depthImage, depthMem, 0);

		auto layoutCmd = static_cast<CommandContext*>(GetDevice()->NewCommandContext(rhi::ECMD_Graphics));
		ImageMemoryBarrierParams params(depthImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		layoutCmd->Begin();
		params.MipLevelCount(1).AspectMask(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT).LayerCount(1);
		layoutCmd->PipelineBarrierImageMemory(params);
		layoutCmd->End();
		layoutCmd->Execute(false);

		VkImageView depthView;
		VkImageViewCreateInfo depthStencilView = {};
		depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = depthFormat;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = DetermineAspectMask(depthFormat);
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;
		depthStencilView.image = depthImage;
		vkCreateImageView(GetRawDevice(), &depthStencilView, nullptr, &depthView);

		m_RenderTargets.resize(m_NumBufferCount);
		RenderpassOptions option = m_RenderPass->GetOption();
		VkFormat colorFmt = option.GetAttachments()[0].GetFormat();
		for (uint32_t i = 0; i < m_NumBufferCount; ++i)
		{
			VkImage colorImage = m_pSwapChain->GetBackImage(i);
			auto colorImageInfo = ImageViewInfo::CreateColorImageView(GetRawDevice(), colorFmt, colorImage);
			VKLOG(Info, "swapchain imageView created . (0x%0x).", colorImageInfo.first);
			colorImageInfo.second.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			auto colorTex = Texture::CreateFromSwapChain(colorImage, colorImageInfo.first, colorImageInfo.second, GetDevice());
			
			//vkTools::setImageLayout(
			//	setupCmdBuffer,
			//	depthStencil.image,
			//	VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			//	VK_IMAGE_LAYOUT_UNDEFINED,
			//	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			FrameBuffer::Attachment colorAttach(colorImageInfo.first);
			FrameBuffer::Attachment depthAttach(depthView);
			FrameBuffer::Option op;
			op.Width = GetWidth();
			op.Height = GetHeight();
			op.Attachments.push_back(colorAttach);
			op.Attachments.push_back(depthAttach);
			auto framebuffer = SpFramebuffer(new FrameBuffer(GetDevice(), m_RenderPass->GetPass(), op));
			m_RenderTargets[i] = std::make_shared<RenderTarget>(GetDevice(), colorTex, framebuffer, m_RenderPass->GetPass());
		}
	}
}

VkRenderPass RenderViewport::GetRenderPass() const { return m_RenderPass->GetPass(); }

K3D_VK_END