#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"
#include "VkUtils.h"
#include "Base/vulkantools.h"

K3D_VK_BEGIN

SwapChain::SwapChain(Device::Ptr pDevice) : DeviceChild(pDevice), m_ReserveBackBufferCount(0)
{
	InitProcs();
}

SwapChain::~SwapChain()
{

}

void SwapChain::Initialize(
	void * WindowHandle, rhi::EPixelFormat const& PixelFormat, 
	uint32 Width, uint32 Height, 
	uint32 * NumBackBuffers, std::vector<VkImage>& OutImages)
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
	K3D_VK_VERIFY(vkCreateAndroidSurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &m_Surface));
#endif
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
	uint32_t formatCount;
	K3D_VK_VERIFY(fpGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), m_Surface, &formatCount, NULL));
	std::vector<VkSurfaceFormatKHR> surfFormats(formatCount);
	K3D_VK_VERIFY(fpGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), m_Surface, &formatCount, surfFormats.data()));
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		colorFormat = /*VK_FORMAT_B8G8R8A8_UNORM*/g_FormatTable[PixelFormat];
	}
	else
	{
		K3D_ASSERT(formatCount >= 1);
		colorFormat = surfFormats[0].format;
	}
	colorSpace = surfFormats[0].colorSpace;
	VkSurfaceCapabilitiesKHR SurfProperties;

	std::vector<VkBool32> queuePresentSupport(GetDevice()->GetQueueCount());

	for (uint32_t i = 0; i < GetDevice()->GetQueueCount(); ++i) 
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(GetPhysicalDevice(), i, m_Surface, &queuePresentSupport[i]);
		if (queuePresentSupport[i]) 
		{
			m_SelectedPresentQueueFamilyIndex = i;
			break;
		}
	}

	K3D_VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetPhysicalDevice(), m_Surface, &SurfProperties));
	uint32 desiredNumBuffers = kMath::Clamp(*NumBackBuffers, SurfProperties.minImageCount, SurfProperties.maxImageCount);
	*NumBackBuffers = desiredNumBuffers;
	m_ReserveBackBufferCount = desiredNumBuffers;
	VkSwapchainCreateInfoKHR swapchainCI = {};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.pNext = NULL;
	swapchainCI.surface = m_Surface;
	swapchainCI.minImageCount = desiredNumBuffers;
	swapchainCI.imageFormat = colorFormat;
	swapchainCI.imageColorSpace = colorSpace;
	swapchainCI.imageExtent = { Width, Height };
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)SurfProperties.currentTransform;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.queueFamilyIndexCount = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.pQueueFamilyIndices = NULL;
	swapchainCI.presentMode = swapchainPresentMode;
	swapchainCI.oldSwapchain = VK_NULL_HANDLE;
	swapchainCI.clipped = true;
	swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	K3D_VK_VERIFY(fpCreateSwapchainKHR(GetRawDevice(), &swapchainCI, nullptr, &m_SwapChain));

	uint32 NumSwapChainImages;
	K3D_VK_VERIFY(fpGetSwapchainImagesKHR(GetRawDevice(), m_SwapChain, &NumSwapChainImages, nullptr));

	OutImages.resize(NumSwapChainImages);
	K3D_VK_VERIFY(fpGetSwapchainImagesKHR(GetRawDevice(), m_SwapChain, &NumSwapChainImages, OutImages.data()));
}

uint32 SwapChain::AcquireNextImage(PtrSemaphore presentSemaphore, PtrFence pFence)
{
	uint32 imageIndex;
	VkResult result = vkAcquireNextImageKHR(GetRawDevice(), m_SwapChain, UINT64_MAX, 
		presentSemaphore->m_Semaphore, pFence? pFence->m_Fence : VK_NULL_HANDLE,
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
	VkPresentInfoKHR present_info = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1,
		&renderingFinishSemaphore->m_Semaphore,
		1,
		&m_SwapChain,
		&imageIndex,
		nullptr
	};
	VkResult result = vkQueuePresentKHR(GetDevice()->GetRawDeviceQueue(), &present_info);
	switch (result) {
	case VK_SUCCESS:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
	case VK_SUBOPTIMAL_KHR:
		//OnWindowSizeChanged();
	default:
		break;
	}
}

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

RenderViewport::RenderViewport(rhi::IDevice * pDevice, void * windowHandle, uint32 width, uint32 height)
	: m_pSwapChain(new SwapChain(static_cast<Device::Ptr>(pDevice)))
	, m_NumBufferCount{2}
	, DeviceChild(static_cast<Device::Ptr>(pDevice))
	, m_CurFrameId(0)
{
	m_pSwapChain->Initialize(
		windowHandle, 
		rhi::EPixelFormat::EPF_RGBA8Unorm, 
		width, height, 
		&m_NumBufferCount, m_SwapChainImages);
	InitializePresentContext(m_NumBufferCount);
	
	Log::Out(LogLevel::Info, "RenderViewport", "Vk_Initialized: width(%d), height(%d), swapImage(%d).",
		width, height, m_SwapChainImages.size());
	
	CreateDefaultPass();
	
	m_PresentSemaphore = GetDevice()->NewSemaphore();
	m_RenderSemaphore = GetDevice()->NewSemaphore();
}

RenderViewport::~RenderViewport()
{
	Log::Out(LogLevel::Info, "RenderViewport", "Vk_Destroyed..");
}

bool RenderViewport::InitViewport(	
	void *windowHandle, rhi::IDevice * pDevice,
	uint32 width, uint32 height, rhi::EPixelFormat rtFmt)
{
	RecordCmdBuffers();
	return true;
}

void RenderViewport::InitFrameBuffers(VkRenderPass renderPass, VkImageView depthView)
{
	for (uint32 i = 0; i < m_SwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo viewCreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
		};
		VkImageView view;
		K3D_VK_VERIFY(vkCreateImageView(GetRawDevice(), &viewCreateInfo, nullptr, &view));
		VkImageView attachments[2] = {
			view, depthView,
		};

		FrameBuffer::Option op;
		new FrameBuffer(GetDevice(), renderPass, op);
	}
}

/* For test */
void RenderViewport::RecordCmdBuffers()
{
	Log::Out(LogLevel::Debug, "RenderViewport", "#RecordCmdBuffers..");
	VkClearColorValue clear_color = {
		{ 1.0f, 0.8f, 0.4f, 0.0f }
	};

	VkImageSubresourceRange image_subresource_range = {
		VK_IMAGE_ASPECT_COLOR_BIT,                    // VkImageAspectFlags                     aspectMask
		0,                                            // uint32_t                               baseMipLevel
		1,                                            // uint32_t                               levelCount
		0,                                            // uint32_t                               baseArrayLayer
		1                                             // uint32_t                               layerCount
	};

	for (uint32_t i = 0; i < m_NumBufferCount; ++i) {
		VkImageMemoryBarrier barrier_from_present_to_clear = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
			nullptr,                                    // const void                            *pNext
			VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          srcAccessMask
			VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
			VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                          oldLayout
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          newLayout
			m_pSwapChain->GetPresentQueueFamilyIndex(),             // uint32_t                               srcQueueFamilyIndex
			m_pSwapChain->GetPresentQueueFamilyIndex(),             // uint32_t                               dstQueueFamilyIndex
			m_SwapChainImages[i],                       // VkImage                                image
			image_subresource_range                     // VkImageSubresourceRange                subresourceRange
		};

		VkImageMemoryBarrier barrier_from_clear_to_present = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
			nullptr,                                    // const void                            *pNext
			VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          srcAccessMask
			VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          dstAccessMask
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          oldLayout
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                          newLayout
			m_pSwapChain->GetPresentQueueFamilyIndex(),             // uint32_t                               srcQueueFamilyIndex
			m_pSwapChain->GetPresentQueueFamilyIndex(),             // uint32_t                               dstQueueFamilyIndex
			m_SwapChainImages[i],                       // VkImage                                image
			image_subresource_range                     // VkImageSubresourceRange                subresourceRange
		};

		m_PresentContext[i]->Begin();
		m_PresentContext[i]->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);
		m_PresentContext[i]->ClearColorImage(m_SwapChainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_subresource_range);
		m_PresentContext[i]->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);
		m_PresentContext[i]->End();
	}
}

void RenderViewport::PrepareNextFrame()
{
	m_CurFrameId = m_pSwapChain->AcquireNextImage(m_PresentSemaphore, nullptr);
	m_PresentContext[m_CurFrameId]->SubmitAndWait(m_PresentSemaphore, m_RenderSemaphore, nullptr);
}

bool RenderViewport::Present(bool vSync)
{
	m_pSwapChain->Present(m_CurFrameId, m_RenderSemaphore);
	return false;
}

void RenderViewport::CreateDefaultPass()
{
	VkFormat colorformat = VK_FORMAT_B8G8R8A8_UNORM;

	VkAttachmentDescription attachments[2];
	attachments[0].format = colorformat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkFormat depthFormat;
	VkBool32 validDepthFormat = vkTools::getSupportedDepthFormat(GetPhysicalDevice(), &depthFormat);

	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depthReference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = NULL;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	K3D_VK_VERIFY(vkCreateRenderPass(GetRawDevice(), &renderPassInfo, nullptr, &renderPass));

	GetDevice()->PushRenderPass(renderPass);

	Log::Out(LogLevel::Info, "RenderViewport", "Default RenderPass created..");
}

void RenderViewport::InitializePresentContext(uint32 Count)
{
	for (uint32 i = 0; i < Count; ++i)
	{
		auto context = GetDevice()->NewCommandContext(rhi::ECMD_Graphics);
		m_PresentContext.push_back(std::static_pointer_cast<CommandContext>
			(std::shared_ptr<rhi::ICommandContext>(context)));
	}
}

K3D_VK_END