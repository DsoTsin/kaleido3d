#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkUtils.h"

K3D_VK_BEGIN

VkDeviceSize CalcAlignedOffset(VkDeviceSize offset, VkDeviceSize align)
{
	VkDeviceSize n = offset / align;
	VkDeviceSize r = offset % align;
	VkDeviceSize result = (n + (r > 0 ? 1 : 0)) * align;
	return result;
}

VkImageAspectFlags DetermineAspectMask(VkFormat format)
{
	VkImageAspectFlags result = 0;
	switch (format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
	{
		result = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	break;
	case VK_FORMAT_S8_UINT:
	{
		result = VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	break;
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
	{
		result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	break;
	default:
	{
		result = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	break;
	}
	return result;
}

std::string ErrorString(VkResult errorCode)
{
	switch (errorCode)
	{
#define VK_RES_STR(r) case VK_ ##r: return #r
		VK_RES_STR(NOT_READY);
		VK_RES_STR(TIMEOUT);
		VK_RES_STR(EVENT_SET);
		VK_RES_STR(EVENT_RESET);
		VK_RES_STR(INCOMPLETE);
		VK_RES_STR(ERROR_OUT_OF_HOST_MEMORY);
		VK_RES_STR(ERROR_OUT_OF_DEVICE_MEMORY);
		VK_RES_STR(ERROR_INITIALIZATION_FAILED);
		VK_RES_STR(ERROR_DEVICE_LOST);
		VK_RES_STR(ERROR_MEMORY_MAP_FAILED);
		VK_RES_STR(ERROR_LAYER_NOT_PRESENT);
		VK_RES_STR(ERROR_EXTENSION_NOT_PRESENT);
		VK_RES_STR(ERROR_INCOMPATIBLE_DRIVER);
#undef VK_RES_STR
	default:
		return "UNKNOWN_ERROR";
	}
}

VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat * depthFormat)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = format;
			return true;
		}
	}

	return false;
}

rhi::PipelineLayoutKey HashPipelineLayoutDesc(rhi::PipelineLayoutDesc const & desc)
{
	rhi::PipelineLayoutKey key;
	key.BindingKey = util::Hash32((const char*)desc.Bindings.Data(),
		desc.Bindings.Count()*sizeof(shaderbinding::Binding));
	key.SetKey = util::Hash32((const char*)desc.Sets.Data(),
		desc.Sets.Count()*sizeof(shaderbinding::Set));
	key.UniformKey = util::Hash32((const char*)desc.Uniforms.Data(),
		desc.Uniforms.Count()*sizeof(shaderbinding::Uniform));
	return key;
}

PtrCmdAlloc CommandAllocator::CreateAllocator(uint32 queueFamilyIndex, bool transient, Device::Ptr device)
{
	PtrCmdAlloc result = PtrCmdAlloc(new CommandAllocator(queueFamilyIndex, transient, device));
	return result;
}

CommandAllocator::~CommandAllocator()
{
	Destroy();
}

void CommandAllocator::Initialize()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.queueFamilyIndex = m_FamilyIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (m_Transient)
	{
		createInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	}

	K3D_VK_VERIFY(vkCreateCommandPool(GetRawDevice(), &createInfo, nullptr, &m_Pool));
}

void CommandAllocator::Destroy()
{
	if (VK_NULL_HANDLE == m_Pool)
	{
		return;
	}
	vkDestroyCommandPool(GetRawDevice(), m_Pool, nullptr);
	m_Pool = VK_NULL_HANDLE;
}

CommandAllocator::CommandAllocator(uint32 queueFamilyIndex, bool transient, Device::Ptr device)
	: m_FamilyIndex(queueFamilyIndex)
	, m_Transient(transient)
	, DeviceChild(device)
{
	Initialize();
}

RenderTarget::RenderTarget(Device::Ptr pDevice, rhi::RenderTargetLayout const & Layout)
	: DeviceChild(pDevice)
{
}

RenderTarget::RenderTarget(Device::Ptr pDevice, SpTexture texture, SpFramebuffer framebuffer, VkRenderPass renderpass)
	: DeviceChild(pDevice)
	, m_RenderTexture(texture)
	, m_Framebuffer(framebuffer)
	, m_Renderpass(renderpass)
{
	m_AcquireSemaphore = PtrSemaphore(new Semaphore(pDevice));
}

RenderTarget::~RenderTarget()
{
}

VkFramebuffer RenderTarget::GetFramebuffer() const
{
	return m_Framebuffer->Get();
}

VkRenderPass RenderTarget::GetRenderpass() const
{
	return m_Renderpass;
}

SpTexture RenderTarget::GetTexture() const
{
	return m_RenderTexture;
}

VkRect2D RenderTarget::GetRenderArea() const
{
	VkRect2D renderArea = {};
	renderArea.offset = { 0, 0 };
	renderArea.extent = { m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight() };
	return renderArea;
}

rhi::IGpuResource * RenderTarget::GetBackBuffer()
{
	return m_RenderTexture.get();
}


K3D_VK_END

