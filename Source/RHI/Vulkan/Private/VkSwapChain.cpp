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
	/*gfxSetting.Width = m_SwapchainExtent.width;
	gfxSetting.Height = m_SwapchainExtent.height;*/
	uint32 desiredNumBuffers = kMath::Clamp(
		gfxSetting.BackBufferCount, 
		surfProperties.minImageCount, 
		surfProperties.maxImageCount);
	m_DesiredBackBufferCount = desiredNumBuffers;
	InitSwapChain(m_DesiredBackBufferCount, chosenFormat, swapchainPresentMode, surfProperties.currentTransform);
	K3D_VK_VERIFY(fpGetSwapchainImagesKHR(GetRawDevice(), m_SwapChain, &m_ReserveBackBufferCount, nullptr));
	m_ColorImages.resize(m_ReserveBackBufferCount);
	K3D_VK_VERIFY(fpGetSwapchainImagesKHR(GetRawDevice(), m_SwapChain, &m_ReserveBackBufferCount, m_ColorImages.data()));
	gfxSetting.BackBufferCount = m_ReserveBackBufferCount;
	VKLOG(Info, "[SwapChain::Initialize] desired imageCount=%d, reserved imageCount = %d.", m_DesiredBackBufferCount, m_ReserveBackBufferCount);
}

uint32 SwapChain::AcquireNextImage(PtrSemaphore presentSemaphore, PtrFence pFence)
{
	uint32 imageIndex;
	VkResult result = fpAcquireNextImageKHR(GetRawDevice(), m_SwapChain, UINT64_MAX,
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
		VKLOG(Info, "Swapchain need update");
	default:
		break;
	}
	return imageIndex;
}

VkResult SwapChain::Present(uint32 imageIndex, PtrSemaphore renderingFinishSemaphore)
{
	VkSemaphore renderSem = renderingFinishSemaphore? renderingFinishSemaphore->GetNativeHandle(): VK_NULL_HANDLE;
	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr };
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.swapchainCount = m_SwapChain ? 1 : 0;
	presentInfo.pSwapchains = &m_SwapChain;
	presentInfo.waitSemaphoreCount = renderSem ? 1 : 0;
	presentInfo.pWaitSemaphores = &renderSem;
	return fpQueuePresentKHR(GetImmCmdQueue()->GetNativeHandle(), &presentInfo);
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
		colorFormat = g_FormatTable[gfxSetting.ColorFormat];
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
	VkSwapchainCreateInfoKHR swapchainCI = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainCI.surface = m_Surface;
	swapchainCI.minImageCount = numBuffers;
	swapchainCI.imageFormat = color.first;
	m_ColorAttachFmt = color.first;
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
	VKLOG(Info, "Init Swapchain with ColorFmt(%d)", m_ColorAttachFmt);
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

K3D_VK_END