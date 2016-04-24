#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"
#include "VkUtils.h"

K3D_VK_BEGIN

SwapChain::SwapChain(Device::Ptr pDevice) : DeviceChild(pDevice), m_ReserveBackBufferCount(0)
{
	InitProcs();
}

SwapChain::~SwapChain()
{
	Destroy();
}

void SwapChain::Initialize(void * WindowHandle, rhi::GfxSetting & gfxSetting)
{
	InitSurface(WindowHandle);
	VkPresentModeKHR swapchainPresentMode				= ChoosePresentMode();
	std::pair<VkFormat, VkColorSpaceKHR> chosenFormat	= ChooseFormat(gfxSetting);
	m_SelectedPresentQueueFamilyIndex					= ChooseQueueIndex();
	m_SwapchainExtent = { gfxSetting.Width, gfxSetting.Height };
	VkSurfaceCapabilitiesKHR surfProperties;
	K3D_VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetPhysicalDevice(), m_Surface, &surfProperties));
	m_SwapchainExtent = surfProperties.currentExtent;
	uint32 desiredNumBuffers = kMath::Clamp(
		gfxSetting.BackBufferCount, 
		surfProperties.minImageCount, 
		surfProperties.maxImageCount);
	gfxSetting.BackBufferCount = desiredNumBuffers;
	m_ReserveBackBufferCount = desiredNumBuffers;
	InitSwapChain(
		m_ReserveBackBufferCount,
		chosenFormat, 
		swapchainPresentMode, 
		surfProperties.currentTransform);
	uint32 numSwapChainImages;
	K3D_VK_VERIFY(fpGetSwapchainImagesKHR(GetRawDevice(), m_SwapChain, &numSwapChainImages, nullptr));
	m_ColorImages.resize(numSwapChainImages);
	K3D_VK_VERIFY(fpGetSwapchainImagesKHR(GetRawDevice(), m_SwapChain, &numSwapChainImages, m_ColorImages.data()));
	VKLOG(Info, "[%s] num images = %d.", __K3D_FUNC__, numSwapChainImages);
}

uint32 SwapChain::AcquireNextImage(PtrSemaphore presentSemaphore, PtrFence pFence)
{
	uint32 imageIndex;
	VkResult result = vkAcquireNextImageKHR(GetRawDevice(), m_SwapChain, UINT64_MAX,
		presentSemaphore ? presentSemaphore->m_Semaphore:VK_NULL_HANDLE, 
		pFence ? pFence->m_Fence : VK_NULL_HANDLE,
		&imageIndex);
	switch (result)
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		//OnWindowSizeChanged();
	default:
		break;
	}
	return imageIndex;
}

void SwapChain::Present(uint32 imageIndex, PtrSemaphore renderingFinishSemaphore)
{
	GetDevice()->GetDefaultCmdQueue()->Present(renderingFinishSemaphore->m_Semaphore, m_SwapChain, imageIndex);
}

void SwapChain::InitSurface(void * WindowHandle)
{
#if K3DPLATFORM_OS_WIN
	VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
	SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	SurfaceCreateInfo.hwnd = (HWND)WindowHandle;
	K3D_VK_VERIFY(vkCreateWin32SurfaceKHR(RHIRoot::GetInstance(), &SurfaceCreateInfo, nullptr, &m_Surface));
#elif K3DPLATFORM_OS_ANDROID
	VkAndroidSurfaceCreateInfoKHR SurfaceCreateInfo = {};
	SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	SurfaceCreateInfo.window = (ANativeWindow*)WindowHandle;
	K3D_VK_VERIFY(vkCreateAndroidSurfaceKHR(RHIRoot::GetInstance(), &SurfaceCreateInfo, nullptr, &m_Surface));
#endif
}

VkPresentModeKHR SwapChain::ChoosePresentMode()
{
	uint32_t presentModeCount;
	K3D_VK_VERIFY(fpGetPhysicalDeviceSurfacePresentModesKHR(GetPhysicalDevice(), m_Surface, &presentModeCount, NULL));
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	K3D_VK_VERIFY(fpGetPhysicalDeviceSurfacePresentModesKHR(GetPhysicalDevice(), m_Surface, &presentModeCount, presentModes.data()));
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}
	return swapchainPresentMode;
}

std::pair<VkFormat, VkColorSpaceKHR> SwapChain::ChooseFormat(rhi::GfxSetting & gfxSetting)
{
	uint32_t formatCount;
	K3D_VK_VERIFY(fpGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), m_Surface, &formatCount, NULL));
	std::vector<VkSurfaceFormatKHR> surfFormats(formatCount);
	K3D_VK_VERIFY(fpGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), m_Surface, &formatCount, surfFormats.data()));
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		colorFormat = /*VK_FORMAT_B8G8R8A8_UNORM*/g_FormatTable[gfxSetting.ColorFormat];
	}
	else
	{
		K3D_ASSERT(formatCount >= 1);
		colorFormat = surfFormats[0].format;
	}
	colorSpace = surfFormats[0].colorSpace;
	return std::make_pair(colorFormat, colorSpace);
}

int SwapChain::ChooseQueueIndex()
{
	uint32 chosenIndex = 0;
	std::vector<VkBool32> queuePresentSupport(GetDevice()->GetQueueCount());

	for (uint32_t i = 0; i < GetDevice()->GetQueueCount(); ++i)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(GetPhysicalDevice(), i, m_Surface, &queuePresentSupport[i]);
		if (queuePresentSupport[i])
		{
			chosenIndex = i;
			break;
		}
	}

	return chosenIndex;
}

void SwapChain::InitSwapChain(uint32 numBuffers, std::pair<VkFormat, VkColorSpaceKHR> color, VkPresentModeKHR mode, VkSurfaceTransformFlagBitsKHR pretran)
{
	VkSwapchainCreateInfoKHR swapchainCI = {};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.pNext = NULL;
	swapchainCI.surface = m_Surface;
	swapchainCI.minImageCount = numBuffers;
	swapchainCI.imageFormat = color.first;
	swapchainCI.imageColorSpace = color.second;
	swapchainCI.imageExtent = m_SwapchainExtent;
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCI.preTransform = pretran;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.queueFamilyIndexCount = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.pQueueFamilyIndices = NULL;
	swapchainCI.presentMode = mode;
	swapchainCI.oldSwapchain = VK_NULL_HANDLE;
	swapchainCI.clipped = true;
	swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	K3D_VK_VERIFY(fpCreateSwapchainKHR(GetRawDevice(), &swapchainCI, nullptr, &m_SwapChain));
}

//void SwapChain::InitBackRenderTargets(rhi::GfxSetting & gfxSetting, VkFormat colorFmt)
//{

	// Back RenderTarget
	//m_RenderTargets.resize(numSwapChainImages);

	//auto renderPass = GetDevice()->GetTopPass();
	//auto rpOptions = renderPass->GetOption();
	//bool hasDepthStencilTarget = false;
	//for (auto attach : rpOptions.GetAttachments())
	//{
	//	if (attach.GetFinalLayout() == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	//	{
	//		hasDepthStencilTarget = true;
	//		break;
	//	}
	//}

	//for (uint32_t i = 0; i < numSwapChainImages; ++i)
	//{
	//	VkImageView imageView = VK_NULL_HANDLE;
	//	ImageViewInfo info = ImageViewInfo::CreateColorImage(colorFmt, m_ColorImages[i]);
	//	K3D_ASSERT(0 != info.subresourceRange.aspectMask);
	//	if (VK_IMAGE_ASPECT_COLOR_BIT == info.subresourceRange.aspectMask)
	//	{
	//		info.components.r = VK_COMPONENT_SWIZZLE_R;
	//		info.components.g = VK_COMPONENT_SWIZZLE_G;
	//		info.components.b = VK_COMPONENT_SWIZZLE_B;
	//		info.components.a = VK_COMPONENT_SWIZZLE_A;
	//	}
	//	VKLOG(Info, "swapchain image created . (0x%0x).", colorBuffers[i]);
	//	K3D_VK_VERIFY(vkCreateImageView(GetRawDevice(), &info, nullptr, &imageView));
	//	// Create Color Texture
	//	auto colorTex = Texture::CreateFromSwapChain(colorBuffers[i], imageView, info, GetDevice());
	//	FrameBuffer::Attachment attch(imageView);
	//	FrameBuffer::Option op;
	//	op.Width = gfxSetting.Width;
	//	op.Height = gfxSetting.Height;
	//	op.Attachments.push_back(attch);
	//	//op.Attachments.push_back(depthView);
	//	auto framebuffer = SpFramebuffer(new FrameBuffer(GetDevice(), renderPass->GetPass(), op));
	//	m_RenderTargets[i] = std::make_shared<RenderTarget>(GetDevice(), colorTex, framebuffer, GetDevice()->GetTopPass());

	//	/*auto cmd = GetDevice()->NewCommandContext(rhi::ECMD_Graphics);
	//	cmd->Begin();
	//	ImageMemoryBarrierParams param(colorBuffers[i], 
	//		VK_IMAGE_LAYOUT_UNDEFINED, 
	//		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	//	param.LayerCount(1).AspectMask(VK_IMAGE_ASPECT_COLOR_BIT).MipLevelCount(1)
	//		.SrcStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT).DstStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
	//	static_cast<CommandContext*>(cmd)->PipelineBarrierImageMemory(param);
	//	cmd->End();
	//	cmd->Execute(false);*/
	//}
//}

void SwapChain::Destroy()
{
	if (m_SwapChain)
	{
		fpDestroySwapchainKHR(GetRawDevice(), m_SwapChain, nullptr);
		m_SwapChain = VK_NULL_HANDLE;
	}
	if (m_Surface)
	{
		vkDestroySurfaceKHR(RHIRoot::GetInstance(), m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;
	}
}

K3D_VK_END