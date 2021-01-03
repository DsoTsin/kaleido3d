#include "vk_common.h"
#include "vk_format.h"

namespace vulkan {
	GpuPresentLayer::GpuPresentLayer(GpuDevice* device, VkSurfaceKHR surface, VkSwapchainKHR swapchain,
									 VkFormat imageFormat, uint32_t width, uint32_t height)
        : surface_(surface)
		, swapchain_(swapchain)
		, image_format_(imageFormat)
		, width_(width)
		, height_(height)
		, device_(device)
    {
		uint32_t imageCount = 0;
		device->getSwapchainImages(swapchain, &imageCount, nullptr);
		swapchain_images_.resize(imageCount);
		device->getSwapchainImages(swapchain, &imageCount, &swapchain_images_[0]);

		for (uint32_t i = 0; i < imageCount; ++i) {
			VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.image = swapchain_images_[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = imageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VkImageView view = VK_NULL_HANDLE;
			device->createImageView(&viewInfo, &view);
			swapchain_views_.push(view);
		}

		VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		device->createSemaphore(semInfo, &image_available_sem_);
		device->createSemaphore(semInfo, &render_finished_sem_);

		device->__GetDeviceQueue(device->getVkDevice(), 
			device->queues_info_.presentQueueFamilyIndex, 0, &present_queue_);
    }

	GpuPresentLayer::~GpuPresentLayer()
    {
		if (image_available_sem_ != VK_NULL_HANDLE) {
			device_->destroySemaphore(image_available_sem_);
			image_available_sem_ = VK_NULL_HANDLE;
		}
		if (render_finished_sem_ != VK_NULL_HANDLE) {
			device_->destroySemaphore(render_finished_sem_);
			render_finished_sem_ = VK_NULL_HANDLE;
		}

		for (size_t i = 0; i < swapchain_views_.num(); ++i) {
			if (swapchain_views_[i] != VK_NULL_HANDLE)
				device_->destroyImageView(swapchain_views_[i]);
		}
		swapchain_views_.clear();

		if (swapchain_ != VK_NULL_HANDLE)
		{
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
		if (!desc) return;
		desc->format = vkFormatToPixel(image_format_);
		desc->width = width_;
		desc->height = height_;
		desc->colorSpace = ngfx::ColorSpace::SRGBNonLinear;
		desc->hdrDisplay = false;
		desc->maxImages = (ngfx::uint32)swapchain_images_.num();
	}

	ngfx::Device* GpuPresentLayer::device()
	{
		return device_.get();
	}

	ngfx::Drawable* GpuPresentLayer::nextDrawable()
	{
		uint32_t imageIndex = 0;
		VkResult ret = device_->acquireNextImage(swapchain_, UINT64_MAX, 
			image_available_sem_, VK_NULL_HANDLE, &imageIndex);
		if (ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR)
			return nullptr;

		current_image_index_ = imageIndex;
		return new GpuDrawable(this, imageIndex, 
			swapchain_images_[imageIndex], swapchain_views_[imageIndex]);
	}

	void GpuPresentLayer::presentDrawable(uint32_t imageIndex)
	{
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		if (render_finished_sem_ != VK_NULL_HANDLE) {
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &render_finished_sem_;
		}
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain_;
		presentInfo.pImageIndices = &imageIndex;
		device_->queuePresent(present_queue_, &presentInfo);
	}

	GpuDrawable::GpuDrawable(GpuPresentLayer* layer, uint32_t imageIndex, VkImage image, VkImageView view)
		: layer_(layer)
		, image_index_(imageIndex)
		, image_(image)
		, view_(view)
		, texture_wrapper_(nullptr)
	{
	}

	GpuDrawable::~GpuDrawable()
	{
		delete texture_wrapper_;
	}

	int GpuDrawable::drawableId() const { return (int)image_index_; }

	ngfx::PresentLayer* GpuDrawable::layer() { return layer_.get(); }

	ngfx::Texture* GpuDrawable::texture()
	{
		if (!texture_wrapper_) {
			ngfx::PresentLayerDesc desc = {};
			layer_->getDesc(&desc);
			ngfx::TextureDesc texDesc = {};
			texDesc.format = desc.format;
			texDesc.width = desc.width;
			texDesc.height = desc.height;
			texDesc.depth = 1;
			texDesc.layers = 1;
			texDesc.mipLevels = 1;
			texDesc.usages = ngfx::TextureUsage::RenderTarget;
			auto* device = static_cast<GpuDevice*>(layer_->device());
			texture_wrapper_ = new GpuTexture(texDesc, image_, device);
		}
		return texture_wrapper_;
	}

	void GpuDrawable::present()
	{
		layer_->presentDrawable(image_index_);
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

		GpuDevice* vkdevice = static_cast<GpuDevice*>(device);

		ngfx::Vec<VkQueueFamilyProperties> queueProps;
		uint32_t queue_family_count = 0;
		__GetPhysicalDeviceQueueFamilyProperties(vkdevice->physical_device_, &queue_family_count, nullptr);
		queueProps.resize(queue_family_count);
		__GetPhysicalDeviceQueueFamilyProperties(vkdevice->physical_device_, &queue_family_count, &queueProps[0]);

		uint32_t presentQueueFamilyIndex = (uint32_t)-1;
		for (uint32_t i = 0; i < queue_family_count; i++)
		{
#if _WIN32
			VkBool32 win32support = __GetPhysicalDeviceWin32PresentationSupportKHR(vkdevice->physical_device_, i);
#endif
			VkBool32 surfaceSupport = VK_FALSE;
			code = __GetPhysicalDeviceSurfaceSupportKHR(vkdevice->physical_device_, i, surface, &surfaceSupport);
			if (surfaceSupport == VK_TRUE)
			{
				presentQueueFamilyIndex = i;
				break;
			}
		}
		check(presentQueueFamilyIndex != (uint32_t)-1);

		uint32_t formatCount = 0;
		__GetPhysicalDeviceSurfaceFormatsKHR(vkdevice->physical_device_, surface, &formatCount, nullptr);
		ngfx::Vec<VkSurfaceFormatKHR> surfaceFormats;
		surfaceFormats.resize(formatCount);
		__GetPhysicalDeviceSurfaceFormatsKHR(vkdevice->physical_device_, surface, &formatCount, &surfaceFormats[0]);
		check(formatCount > 0);
		
		VkFormat chosenFormat = surfaceFormats[0].format;
		VkColorSpaceKHR chosenColorSpace = surfaceFormats[0].colorSpace;

		VkFormat desiredFormat = pixelFormatToVk(desc->format);
		for (size_t i = 0; i < surfaceFormats.num(); ++i) {
			if (surfaceFormats[i].format == desiredFormat) {
				chosenFormat = surfaceFormats[i].format;
				chosenColorSpace = surfaceFormats[i].colorSpace;
				break;
			}
		}

		VkSwapchainCreateInfoKHR create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		create_info.surface = surface;
		create_info.minImageCount = desc->maxImages > 0 ? desc->maxImages : 3;
		create_info.imageFormat = chosenFormat;
		create_info.imageColorSpace = chosenColorSpace;
		create_info.imageExtent.width = desc->width;
		create_info.imageExtent.height = desc->height;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 1;
		create_info.pQueueFamilyIndices = &presentQueueFamilyIndex;
        create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		code = vkdevice->createSwapchain(&create_info, &swapchain);
		check(code == VK_SUCCESS);

		if (result) *result = (code == VK_SUCCESS) ? ngfx::Result::Ok : ngfx::Result::Failed;

		if (__SetHdrMetadataEXT && desc->hdrDisplay)
		{
			VkHdrMetadataEXT hdrMetadata = { VK_STRUCTURE_TYPE_HDR_METADATA_EXT };
			__SetHdrMetadataEXT(vkdevice->device_, 1, &swapchain, &hdrMetadata);
		}
		return new GpuPresentLayer(vkdevice, surface, swapchain, chosenFormat, desc->width, desc->height);
	}
}
