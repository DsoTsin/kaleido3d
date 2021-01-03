

#include "vk_common.h"

namespace vulkan {
	GpuPresentLayer::GpuPresentLayer(GpuDevice* device, VkSurfaceKHR surface, VkSwapchainKHR swapchain)
        : surface_(surface)
		, swapchain_(swapchain)
		, device_(device)
    {
		uint32_t imageCount = 0;
		device->getSwapchainImages(swapchain, &imageCount, nullptr);
		ngfx::Vec<VkImage> images;
		images.resize(imageCount);
		device->getSwapchainImages(swapchain, &imageCount, &images[0]);
    }

	GpuPresentLayer::~GpuPresentLayer()
    {
		if (swapchain_ != VK_NULL_HANDLE)
		{
			// need destroy image views
			device_->destroySwapchain(swapchain_);
			swapchain_ = VK_NULL_HANDLE;
		}

		if (surface_ != VK_NULL_HANDLE)
		{
			device_->factory_->destroySurface(surface_);
			surface_ = VK_NULL_HANDLE;
		}
    }

	void GpuPresentLayer::getDesc(ngfx::PresentLayerDesc* desc) const
	{
	}

	ngfx::Device* GpuPresentLayer::device()
	{
		return device_.get();
	}

	ngfx::Drawable* GpuPresentLayer::nextDrawable()
	{
		uint64_t timeOut = -1;
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
		uint32_t imageIndex = 0;
		VkResult ret = device_->acquireNextImage(swapchain_, timeOut, semaphore, fence, &imageIndex);

		return nullptr;
	}

	GpuDrawable::GpuDrawable(GpuPresentLayer* layer)
		: layer_(layer)
	{
	}

	GpuDrawable::~GpuDrawable()
	{
	}

	int GpuDrawable::drawableId() const { return image_id_; }

	ngfx::PresentLayer* GpuDrawable::layer() { return layer_.get(); }

	ngfx::Texture* GpuDrawable::texture()
	{
		return nullptr;
	}

	void GpuDrawable::present()
	{

	}

	ngfx::PresentLayer* 
	GpuFactory::newPresentLayer(const ngfx::PresentLayerDesc* desc,
		ngfx::Device* device, ngfx::PresentLayer* old, ngfx::Result* result)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkResult code = VK_SUCCESS;
#if _WIN32
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surfaceCreateInfo.hinstance = (HINSTANCE)desc->extraData;
		surfaceCreateInfo.hwnd = reinterpret_cast<HWND>(desc->winHandle);
		code = __CreateWin32SurfaceKHR(instance_, &surfaceCreateInfo, NGFXVK_ALLOCATOR, &surface);
#elif __ANDROID__

#endif
		check(code == VK_SUCCESS);

		VkBool32 supportPresent = VK_FALSE;
		GpuDevice* vkdevice = static_cast<GpuDevice*>(device);

		ngfx::Vec<VkQueueFamilyProperties> queueProps;
		uint32_t queue_family_count = 0;
		__GetPhysicalDeviceQueueFamilyProperties(vkdevice->physical_device_, &queue_family_count, nullptr);
		queueProps.resize(queue_family_count);
		__GetPhysicalDeviceQueueFamilyProperties(vkdevice->physical_device_, &queue_family_count, &queueProps[0]);

		ngfx::Vec<VkBool32> supportsPresent;
		supportsPresent.resize(queue_family_count);
		uint32_t presentQueueFamilyIndex = -1u;
		for (uint32_t i = 0; i < queue_family_count; i++)
		{
			VkBool32 result = __GetPhysicalDeviceWin32PresentationSupportKHR(vkdevice->physical_device_, i);
			code = __GetPhysicalDeviceSurfaceSupportKHR(vkdevice->physical_device_, i, surface, &supportsPresent[i]);
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueFamilyIndex = i;
				break;
			}
		}
		check(presentQueueFamilyIndex != -1);

		uint32_t formatCount = 0;
		__GetPhysicalDeviceSurfaceFormatsKHR(vkdevice->physical_device_, surface, &formatCount, nullptr);
		ngfx::Vec<VkSurfaceFormatKHR> surfaceFormats;
		surfaceFormats.resize(formatCount);
		__GetPhysicalDeviceSurfaceFormatsKHR(vkdevice->physical_device_, surface, &formatCount, &surfaceFormats[0]);
		check(formatCount > 0);
		
		// TODO: match
		desc->format;
        desc->colorSpace;
		surfaceFormats[0].format;
		surfaceFormats[0].colorSpace;
		/*
		if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			_surfaceFormat.format = _settings.DesiredSurfaceFormat;
			_surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found = false;
			for (auto& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == _settings.DesiredSurfaceFormat)
				{
					_surfaceFormat = surfaceFormat;
					found = true;
					break;
				}
			}
			if (!found)
			{
				_surfaceFormat = surfaceFormats[0];
			}
		}
		*/
		VkSwapchainCreateInfoKHR create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		create_info.surface = surface;
		create_info.minImageCount = 3;
		create_info.imageFormat = surfaceFormats[0].format;
		create_info.imageColorSpace = surfaceFormats[0].colorSpace;
		create_info.imageExtent.width = desc->width;
		create_info.imageExtent.height = desc->height;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT/*VK_IMAGE_USAGE_TRANSFER_SRC_BIT*/;
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 1;
		create_info.pQueueFamilyIndices = &presentQueueFamilyIndex;
        create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		create_info.clipped = VK_FALSE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		code = vkdevice->createSwapchain(&create_info, &swapchain);
		check(code == VK_SUCCESS);
		// need check format is hdr..
		if (__SetHdrMetadataEXT)
		{
			VkHdrMetadataEXT hdrMetadata;
			hdrMetadata.displayPrimaryRed;
			hdrMetadata.displayPrimaryGreen;
			hdrMetadata.displayPrimaryBlue;
			hdrMetadata.whitePoint;
			hdrMetadata.maxLuminance;
			hdrMetadata.minLuminance;
			hdrMetadata.maxContentLightLevel;
			hdrMetadata.maxFrameAverageLightLevel;
			__SetHdrMetadataEXT(vkdevice->device_, 1, &swapchain, &hdrMetadata);
		}
		return new GpuPresentLayer(vkdevice, surface, swapchain);
	}
}