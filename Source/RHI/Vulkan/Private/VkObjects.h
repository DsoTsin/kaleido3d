#ifndef __VkObjects_h__
#define __VkObjects_h__
#pragma once
#include <utility>  

K3D_VK_BEGIN

struct ImageInfo;

class Gpu;
using GpuRef = SharedPtr<Gpu>;

class RenderTargetLayout
{
public:
	RenderTargetLayout(rhi::RenderTargetLayout const & layout);

	inline uint32 GetHash() const { return m_Hash; }
	inline const VkExtent2D& GetExtent2D() const { return m_Extent2D; }
	inline const VkExtent3D& GetExtent3D() const { return m_Extent3D; }
	inline const VkAttachmentDescription* GetAttachmentDescriptions() const { return m_Desc; }
	inline uint32 GetNumColorAttachments() const { return m_NumColorAttachments; }
	inline bool GetHasDepthStencil() const { return m_HasDepthStencil; }
	inline bool GetHasResolveAttachments() const { return m_HasResolveAttachments; }
	inline uint32 GetNumAttachments() const { return m_NumAttachments; }

	inline const VkAttachmentReference* GetColorAttachmentReferences() const { return m_NumColorAttachments > 0 ? m_ColorReferences : nullptr; }
	inline const VkAttachmentReference* GetResolveAttachmentReferences() const { return m_HasResolveAttachments ? m_ResolveReferences : nullptr; }
	inline const VkAttachmentReference* GetDepthStencilAttachmentReference() const { return m_HasDepthStencil ? &m_DepthStencilReference : nullptr; }

private:

	union
	{
		VkExtent3D	m_Extent3D;
		VkExtent2D	m_Extent2D;
	};

	uint32 					m_Hash;
	VkAttachmentReference 	m_ColorReferences[8];
	VkAttachmentReference 	m_ResolveReferences[8];
	VkAttachmentReference 	m_DepthStencilReference;
	VkAttachmentDescription m_Desc[8 * 2 + 1];

	uint32 					m_NumAttachments;
	uint32 					m_NumColorAttachments;
	bool 					m_HasDepthStencil;
	bool 					m_HasResolveAttachments;
};

class K3D_API RenderpassAttachment
{
public:

	struct Description : public VkAttachmentDescription
	{
		Description(VkFormat format, VkSampleCountFlagBits samples)
		{
			format = format;
			samples = samples;
			loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			flags = 0;
		}
		Description() { flags = 0; }
		Description & Format(VkFormat const & fmt) { this->format = fmt; return *this; }
		Description & Samples(VkSampleCountFlagBits const & sample) { this->samples = sample; return *this; }
		Description & LoadOp(VkAttachmentLoadOp const & op) { this->loadOp = op; return *this; }
		Description & StoreOp(VkAttachmentStoreOp const & op) { this->storeOp = op; return *this; }
		Description & StencilLoadOp(VkAttachmentLoadOp const & op) { this->stencilLoadOp = op; return *this; }
		Description & StencilStoreOp(VkAttachmentStoreOp const & op) { this->stencilStoreOp = op; return *this; }
		Description & InitialLayout(VkImageLayout const & layout) { this->initialLayout = layout; return *this; }
		Description & FinalLayout(VkImageLayout const & layout) { this->finalLayout = layout; return *this; }
	};

	RenderpassAttachment(VkFormat fmt = VK_FORMAT_UNDEFINED, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	virtual ~RenderpassAttachment() {}

	RenderpassAttachment&			SetClearValue(const VkClearValue& value) { m_ClearValue = value; return *this; }
	const VkClearValue&				GetClearValue() const { return m_ClearValue; }
	const VkAttachmentDescription&	GetDescription() const { return m_Description; }
	Description&					GetDescription() { return m_Description; }
	VkFormat						GetFormat() const { return m_Description.format; }
	VkImageLayout					GetFinalLayout() const { return m_Description.finalLayout; }

	static RenderpassAttachment	CreateColor(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	static RenderpassAttachment	CreateDepthStencil(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

	friend class			RenderPass;
private:
	Description				m_Description;
	VkClearValue			m_ClearValue;
};

struct K3D_API Subpass
{
	Subpass(VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
		: m_PipelineBindPoint(pipelineBindPoint) {}
	virtual ~Subpass() {}

	const std::vector<uint32>&	GetColorAttachments() const { return m_ColorAttachments; }
	const std::vector<uint32>&	GetDepthStencilAttachment() const { return m_DepthStencilAttachment; }

	Subpass&					AddColorAttachment(uint32 attachmentIndex, uint32 resolveAttachmentIndex = VK_ATTACHMENT_UNUSED);
	Subpass&					AddDepthStencilAttachment(uint32 attachmentIndex);
	Subpass&					AddPreserveAttachment(uint32 attachmentIndex);
	Subpass&					AddPreserveAttachments(const std::vector<uint32>& attachmentIndices);

	struct AttachReferences
	{
		std::vector<VkAttachmentReference>	Color;
		std::vector<VkAttachmentReference>	Resolve;
		std::vector<VkAttachmentReference>	Depth;
		std::vector<uint32>					Preserve;
	};

private:
	VkPipelineBindPoint		m_PipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	std::vector<uint32>		m_ColorAttachments;
	std::vector<uint32>		m_ResolveAttachments;
	std::vector<uint32>		m_DepthStencilAttachment;
	std::vector<uint32>		m_PreserveAttachments;
	friend class RenderPass;
};

class K3D_API SubpassDependency
{
public:
	SubpassDependency(uint32 srcSubpass, uint32 dstSubpass);
	virtual ~SubpassDependency() {}

	SubpassDependency&		SrcSubpass(uint32 subpass) { m_Dependency.srcSubpass = subpass; return *this; }
	SubpassDependency&		DstSubpass(uint32 subpass) { m_Dependency.dstSubpass = subpass; return *this; }
	SubpassDependency&		SrcStageMask(VkPipelineStageFlags mask, bool exclusive = false);
	SubpassDependency&		DstStageMask(VkPipelineStageFlags mask, bool exclusive = false);
	SubpassDependency&		StageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive = false);
	SubpassDependency&		SrcAccessMask(VkAccessFlags mask, bool exclusive = false);
	SubpassDependency&		DstAccessMask(VkAccessFlags mask, bool exclusive = false);
	SubpassDependency&		AccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive = false);

private:
	VkSubpassDependency		m_Dependency;
	friend class			RenderPass;
};

/**
 * depends on RenderpassAttachment, Subpass, SubpassDependency
 */
class K3D_API RenderpassOptions
{
public:
	using Attachments = std::vector<RenderpassAttachment>;
	RenderpassOptions() {}
	RenderpassOptions(VkFormat colorFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	RenderpassOptions(VkFormat colorFormat, VkFormat depthStencilFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	virtual ~RenderpassOptions() {}

	RenderpassOptions&				AddAttachment(const RenderpassAttachment& value) { m_Attachments.push_back(value); return *this; }
	RenderpassOptions&				AddSubPass(const Subpass& value) { m_Subpasses.push_back(value); return *this; }
	RenderpassOptions&				AddSubpassDependency(const SubpassDependency& value) { m_SubpassDependencies.push_back(value); return *this; }
	Attachments const&				GetAttachments() const { return m_Attachments; }

private:
	std::vector<RenderpassAttachment>	m_Attachments;
	std::vector<Subpass>				m_Subpasses;
	std::vector<SubpassDependency>		m_SubpassDependencies;
	friend class RenderPass;
};

class K3D_API BufferMemoryBarrierParams
{
public:
	BufferMemoryBarrierParams(VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask = 0, VkPipelineStageFlags dstStageMask = 0);
	virtual ~BufferMemoryBarrierParams() {}
	BufferMemoryBarrierParams&		SrcAccessMask(VkAccessFlags value, bool exclusive = false) { if (exclusive) { m_Barrier.srcAccessMask = value; } else { m_Barrier.srcAccessMask |= value; } return *this; }
	BufferMemoryBarrierParams&		DstAccessMask(VkAccessFlags value, bool exclusive = false) { if (exclusive) { m_Barrier.dstAccessMask = value; } else { m_Barrier.dstAccessMask |= value; } return *this; }
	BufferMemoryBarrierParams&		SrcQueueFamilyIndex(VkAccessFlags value) { m_Barrier.srcQueueFamilyIndex = value; return *this; }
	BufferMemoryBarrierParams&		DstQueueFamilyIndex(VkAccessFlags value) { m_Barrier.dstQueueFamilyIndex = value; return *this; }
	BufferMemoryBarrierParams&		Offset(VkDeviceSize value) { m_Barrier.offset = value; return *this; }
	BufferMemoryBarrierParams&		Size(VkDeviceSize value) { m_Barrier.size = value; return *this; }
	BufferMemoryBarrierParams&		SrcStageMask(VkPipelineStageFlags value, bool exclusive = false) { if (exclusive) { m_SrcStageMask = value; } else { m_SrcStageMask |= value; } return *this; }
	BufferMemoryBarrierParams&		DstStageMask(VkPipelineStageFlags value, bool exclusive = false) { if (exclusive) { m_DstStageMask = value; } else { m_DstStageMask |= value; } return *this; }
private:
	BufferMemoryBarrierParams();
	VkBufferMemoryBarrier m_Barrier;
	VkPipelineStageFlags m_SrcStageMask = 0;
	VkPipelineStageFlags m_DstStageMask = 0;
	friend class CommandContext;
};

class K3D_API ImageMemoryBarrierParams
{
public:
	ImageMemoryBarrierParams(VkImage image, VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkPipelineStageFlags srcStageMask = 0, VkPipelineStageFlags dstStageMask = 0);

	virtual ~ImageMemoryBarrierParams() {}
	ImageMemoryBarrierParams&		SrcAccessMask(VkAccessFlags value, bool exclusive = false) { if (exclusive) { m_Barrier.srcAccessMask = value; } else { m_Barrier.srcAccessMask |= value; } return *this; }
	ImageMemoryBarrierParams&		DstAccessMask(VkAccessFlags value, bool exclusive = false) { if (exclusive) { m_Barrier.dstAccessMask = value; } else { m_Barrier.dstAccessMask |= value; } return *this; }
	ImageMemoryBarrierParams&		OldLayout(VkImageLayout value) { m_Barrier.oldLayout = value; return *this; }
	ImageMemoryBarrierParams&		NewLayout(VkImageLayout value) { m_Barrier.newLayout = value; return *this; }
	ImageMemoryBarrierParams&		SrcQueueFamilyIndex(VkAccessFlags value) { m_Barrier.srcQueueFamilyIndex = value; return *this; }
	ImageMemoryBarrierParams&		DstQueueFamilyIndex(VkAccessFlags value) { m_Barrier.dstQueueFamilyIndex = value; return *this; }
	ImageMemoryBarrierParams&		AspectMask(VkImageAspectFlags value, bool exclusive = false) { if (exclusive) { m_Barrier.subresourceRange.aspectMask = value; } else { m_Barrier.subresourceRange.aspectMask |= value; } return *this; }
	ImageMemoryBarrierParams&		BaseMipLevel(uint32 value) { m_Barrier.subresourceRange.baseMipLevel = value; return *this; }
	ImageMemoryBarrierParams&		MipLevelCount(uint32 value) { m_Barrier.subresourceRange.levelCount = value; return *this; }
	ImageMemoryBarrierParams&		BaseArrayLayer(uint32 value) { m_Barrier.subresourceRange.baseArrayLayer = value; return *this; }
	ImageMemoryBarrierParams&		LayerCount(uint32 value) { m_Barrier.subresourceRange.layerCount = value; return *this; }
	ImageMemoryBarrierParams&		SubResourceRange(VkImageSubresourceRange range) { m_Barrier.subresourceRange = range; return *this; }
	ImageMemoryBarrierParams&		SrcStageMask(VkPipelineStageFlags value, bool exclusive = false) { if (exclusive) { m_SrcStageMask = value; } else { m_SrcStageMask |= value; } return *this; }
	ImageMemoryBarrierParams&		DstStageMask(VkPipelineStageFlags value, bool exclusive = false) { if (exclusive) { m_DstStageMask = value; } else { m_DstStageMask |= value; } return *this; }
private:
	ImageMemoryBarrierParams();
	VkImageMemoryBarrier m_Barrier;
	VkPipelineStageFlags m_SrcStageMask = 0;
	VkPipelineStageFlags m_DstStageMask = 0;
	friend class CommandContext;
};

struct K3D_API CmdBufAllocInfo : public VkCommandBufferAllocateInfo
{
	CmdBufAllocInfo()
		: CmdBufAllocInfo(
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			VK_NULL_HANDLE,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1)
	{
	}

	CmdBufAllocInfo(
		VkStructureType type,
		const void* next,
		VkCommandPool pool,
		VkCommandBufferLevel lev = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		uint32 bufCnt = 1)
	{
		this->sType = type;
		this->pNext = next;
		this->commandPool = pool;
		this->level = lev;
		this->commandBufferCount = bufCnt;
	}
	/**
	 * @param pool
	 * @param level VK_COMMAND_BUFFER_LEVEL_PRIMARY/VK_COMMAND_BUFFER_LEVEL_SECONDARY
	 * @param buffCount 
	 */
	static VkCommandBufferAllocateInfo Create(VkCommandPool pool,
		VkCommandBufferLevel lev = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		uint32 bufCnt = 1)
	{
		CmdBufAllocInfo info(
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
			nullptr,
			pool, lev, bufCnt);
		return info;
	}

	CmdBufAllocInfo & Type(VkStructureType type)
	{
		this->sType = type;
		return *this;
	}

	CmdBufAllocInfo & Pool(VkCommandPool const & pool)
	{
		this->commandPool = pool;
		return *this;
	}

	CmdBufAllocInfo & Level(VkCommandBufferLevel const & lev)
	{
		this->level = lev;
		return *this;
	}

	CmdBufAllocInfo & Count(uint32 count)
	{
		this->commandBufferCount = count;
		return *this;
	}
};

struct SemaphoreCreateInfo : public VkSemaphoreCreateInfo
{
public:
	static VkSemaphoreCreateInfo Create()
	{
		return
		{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			nullptr,
			0
		};
	}
};

/**
 * If VK_FENCE_CREATE_SIGNALED_BIT is set then the fence is created already signaled, otherwise, the fence is created in an unsignaled state.
 */
class FenceCreateInfo : public VkFenceCreateInfo
{
public:
	static VkFenceCreateInfo Create()
	{
		return
		{
			VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			nullptr,
			0
		};
	}

};

class ImageViewInfo : public VkImageViewCreateInfo
{
public:
	ImageViewInfo()
	{
		sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		pNext = nullptr;
		viewType = VK_IMAGE_VIEW_TYPE_2D;
		flags = 0;
		image = VK_NULL_HANDLE;
		format = VK_FORMAT_UNDEFINED;
		components = {};

		subresourceRange = {};
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
	}
	
	ImageViewInfo(VkFormat format_, VkImage image_, VkImageViewType viewType_ = VK_IMAGE_VIEW_TYPE_2D)
	{
		this->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		this->pNext = nullptr;
		this->viewType = viewType_;
		this->flags = 0;
		this->image = image_;
		this->format = format_;
		components = {};
		subresourceRange = {};
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
	}

	ImageViewInfo& Format(VkFormat fmt)
	{
		this->format = fmt;
		return *this;
	}

	ImageViewInfo& ViewType(VkImageViewType viewType_ = VK_IMAGE_VIEW_TYPE_2D) 
	{ 
		this->viewType = viewType_; 
		return *this;
	}
	
	ImageViewInfo& AspectType(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) 
	{
		this->subresourceRange.aspectMask = aspectMask;
		return *this; 
	}

	ImageViewInfo& Image(VkImage image_)
	{
		this->image = image_;
		return *this;
	}

	static ImageViewInfo CreateColorImageInfo(
		VkFormat format, VkImage image, 
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT)
	{
		ImageViewInfo info(format, image);
		return info.AspectType(aspectMask);
	}

	static std::pair<VkImageView, VkImageViewCreateInfo> CreateColorImageView(
		VkDevice device, VkFormat colorFmt, VkImage colorImage,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
	static std::pair<VkImageView, VkImageViewCreateInfo> CreateColorImageView(
		GpuRef device, VkFormat colorFmt, VkImage colorImage,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

	static ImageViewInfo CreateDepthStencilImageInfo(
		VkFormat format, VkImage image,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
	{
		ImageViewInfo info(format, image);
		return info.AspectType(aspectMask);
	}

	static std::pair<VkImageView, VkImageViewCreateInfo> CreateDepthStencilImageView(
		VkDevice device, VkFormat colorFmt, VkImage colorImage,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

	static ImageViewInfo From(ImageInfo const& info, VkImage image);
}; 

/*
VkStructureType          sType;
const void*              pNext;
VkImageCreateFlags       flags;
VkImageType              imageType;
VkFormat                 format;
VkExtent3D               extent;
uint32_t                 mipLevels;
uint32_t                 arrayLayers;
VkSampleCountFlagBits    samples;
VkImageTiling            tiling;
VkImageUsageFlags        usage;
VkSharingMode            sharingMode;
uint32_t                 queueFamilyIndexCount;
const uint32_t*          pQueueFamilyIndices;
VkImageLayout            initialLayout;
*/
struct ImageInfo : VkImageCreateInfo
{
	ImageInfo()
	{
		sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		pNext = nullptr;
		flags = 0;
		imageType = VK_IMAGE_TYPE_2D;
		format = VK_FORMAT_UNDEFINED;
		extent = {};
		mipLevels = 1;
		arrayLayers = 1;
		samples = VK_SAMPLE_COUNT_1_BIT;
		tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		queueFamilyIndexCount = 0;
		pQueueFamilyIndices = nullptr;
		initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	ImageInfo& Format(VkFormat fmt)
	{
		this->format = fmt;
		return *this;
	}

	ImageInfo& ImageType(VkImageType type)
	{
		this->imageType = type;
		return *this;
	}

	ImageInfo& MipLevel(uint32 level)
	{
		this->mipLevels = level;
		return *this;
	}

	ImageInfo& Layers(uint32 layers)
	{
		this->arrayLayers = layers;
		return *this;
	}

	ImageInfo& SampleBits(VkSampleCountFlagBits bits)
	{
		this->samples = bits;
		return *this;
	}

	ImageInfo& Dimens(uint32 width, uint32 height = 1, uint32 depth = 1)
	{
		this->extent = { width,height,depth };
		if (height == 1)
		{
			this->imageType = VK_IMAGE_TYPE_1D;
		}
		else if (depth == 1) 
		{
			this->imageType = VK_IMAGE_TYPE_2D;
		}
		else
		{
			this->imageType = VK_IMAGE_TYPE_3D;
		}
		return *this;
	}

	ImageInfo& Usage(VkImageUsageFlagBits bits)
	{
		this->usage = bits;
		return *this;
	}

	ImageInfo& Tiling(VkImageTiling tile)
	{
		this->tiling = tile;
		return *this;
	}

	ImageInfo& InitialLayout(VkImageLayout layout)
	{
		this->initialLayout = layout;
		return *this;
	}

	VkImageViewType GuessViewType() const
	{
		VkImageViewType type = VK_IMAGE_VIEW_TYPE_1D;
		if (this->arrayLayers > 1)
		{
			if (this->imageType == VK_IMAGE_TYPE_2D)
			{
				type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			}
			else if (this->imageType == VK_IMAGE_TYPE_1D)
			{
				type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			}
		}
		else
		{
			if (this->imageType == VK_IMAGE_TYPE_2D)
			{
				type = VK_IMAGE_VIEW_TYPE_2D;
			}
			else if (this->imageType == VK_IMAGE_TYPE_1D)
			{
				type = VK_IMAGE_VIEW_TYPE_1D;
			}
			else
			{
				type = VK_IMAGE_VIEW_TYPE_3D;
			}
		}
		return type;
	}

	static ImageInfo FromRHI(rhi::TextureDesc const & desc);
};

class VkObjectAllocator
{
public:

	inline operator VkAllocationCallbacks() const 
	{ 
		VkAllocationCallbacks result;
		result.pUserData = (void*)this; 
//		result.pfnAllocation = VkObjectAllocator::Allocation;
//		result.pfnReallocation = VkObjectAllocator::Reallocation;
//		result.pfnFree = VkObjectAllocator::Free;
		result.pfnInternalAllocation = nullptr; 
		result.pfnInternalFree = nullptr; 
		return result; 
	}; 

private:
	static void* VKAPI_CALL Allocation( void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope); 
	static void* VKAPI_CALL Reallocation( void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope); 
	static void VKAPI_CALL Free( void* pUserData, void* pMemory); 
	void* Allocation( size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
	void* Reallocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope); 
	void Free(void* pMemory);
};

class CommandBufferManager
{
public:
	CommandBufferManager(GpuRef gpu, VkCommandBufferLevel bufferLevel, unsigned graphicsQueueIndex);
	~CommandBufferManager();
	void			Destroy();
	VkCommandBuffer RequestCommandBuffer();
	void			BeginFrame();

private:
	GpuRef							m_Gpu;
	VkCommandPool					m_Pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer>	m_Buffers;
	VkCommandBufferLevel			m_CommandBufferLevel;
	uint32							m_Count = 0;
};

using CmdBufManagerRef = SharedPtr<CommandBufferManager>;

extern ::k3d::DynArray<VkLayerProperties>		gVkLayerProps;
extern ::k3d::DynArray<VkExtensionProperties>	gVkExtProps;

class Instance;

#define __VK_DEVICE_PROC__(name) PFN_vk##name vk##name = NULL

class Gpu
{
public:
	~Gpu();

	VkDevice CreateLogicDevice(bool enableValidation);
	VkBool32 GetSupportedDepthFormat(VkFormat * depthFormat);

	VkResult GetSurfaceSupportKHR(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
	VkResult GetSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
	VkResult GetSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
	VkResult GetSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);

	void DestroyDevice();
	void FreeCommandBuffers(VkCommandPool,uint32,VkCommandBuffer*);
	VkResult CreateCommdPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
	VkResult AllocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);

	Instance* GetInstance() const { return m_Inst; }

	__VK_DEVICE_PROC__(DestroyDevice);
	__VK_DEVICE_PROC__(GetDeviceQueue);
	__VK_DEVICE_PROC__(QueueSubmit);
	__VK_DEVICE_PROC__(QueueWaitIdle);
	__VK_DEVICE_PROC__(QueuePresentKHR);
	__VK_DEVICE_PROC__(DeviceWaitIdle);
	__VK_DEVICE_PROC__(AllocateMemory);
	__VK_DEVICE_PROC__(FreeMemory);
	__VK_DEVICE_PROC__(MapMemory);
	__VK_DEVICE_PROC__(UnmapMemory);
	__VK_DEVICE_PROC__(FlushMappedMemoryRanges);
	__VK_DEVICE_PROC__(InvalidateMappedMemoryRanges);
	__VK_DEVICE_PROC__(GetDeviceMemoryCommitment);
	__VK_DEVICE_PROC__(BindBufferMemory);
	__VK_DEVICE_PROC__(BindImageMemory);
	__VK_DEVICE_PROC__(GetBufferMemoryRequirements);
	__VK_DEVICE_PROC__(GetImageMemoryRequirements);
	__VK_DEVICE_PROC__(GetImageSparseMemoryRequirements);
	__VK_DEVICE_PROC__(QueueBindSparse);
	__VK_DEVICE_PROC__(CreateFence);
	__VK_DEVICE_PROC__(DestroyFence);
	__VK_DEVICE_PROC__(ResetFences);
	__VK_DEVICE_PROC__(GetFenceStatus);
	__VK_DEVICE_PROC__(WaitForFences);
	__VK_DEVICE_PROC__(CreateSemaphore);
	__VK_DEVICE_PROC__(DestroySemaphore);
	__VK_DEVICE_PROC__(CreateEvent);
	__VK_DEVICE_PROC__(DestroyEvent);
	__VK_DEVICE_PROC__(GetEventStatus);
	__VK_DEVICE_PROC__(SetEvent);
	__VK_DEVICE_PROC__(ResetEvent);
	__VK_DEVICE_PROC__(CreateQueryPool);
	__VK_DEVICE_PROC__(DestroyQueryPool);
	__VK_DEVICE_PROC__(GetQueryPoolResults);
	__VK_DEVICE_PROC__(CreateBuffer);
	__VK_DEVICE_PROC__(DestroyBuffer);
	__VK_DEVICE_PROC__(CreateBufferView);
	__VK_DEVICE_PROC__(DestroyBufferView);
	__VK_DEVICE_PROC__(CreateImage);
	__VK_DEVICE_PROC__(DestroyImage);
	__VK_DEVICE_PROC__(GetImageSubresourceLayout);
	__VK_DEVICE_PROC__(CreateImageView);
	__VK_DEVICE_PROC__(DestroyImageView);
	__VK_DEVICE_PROC__(CreateShaderModule);
	__VK_DEVICE_PROC__(DestroyShaderModule);
	__VK_DEVICE_PROC__(CreatePipelineCache);
	__VK_DEVICE_PROC__(DestroyPipelineCache);
	__VK_DEVICE_PROC__(GetPipelineCacheData);
	__VK_DEVICE_PROC__(MergePipelineCaches);
	__VK_DEVICE_PROC__(CreateGraphicsPipelines);
	__VK_DEVICE_PROC__(CreateComputePipelines);
	__VK_DEVICE_PROC__(DestroyPipeline);
	__VK_DEVICE_PROC__(CreatePipelineLayout);
	__VK_DEVICE_PROC__(DestroyPipelineLayout);
	__VK_DEVICE_PROC__(CreateSampler);
	__VK_DEVICE_PROC__(DestroySampler);
	__VK_DEVICE_PROC__(CreateDescriptorSetLayout);
	__VK_DEVICE_PROC__(DestroyDescriptorSetLayout);
	__VK_DEVICE_PROC__(CreateDescriptorPool);
	__VK_DEVICE_PROC__(DestroyDescriptorPool);
	__VK_DEVICE_PROC__(ResetDescriptorPool);
	__VK_DEVICE_PROC__(AllocateDescriptorSets);
	__VK_DEVICE_PROC__(FreeDescriptorSets);
	__VK_DEVICE_PROC__(UpdateDescriptorSets);
	__VK_DEVICE_PROC__(CreateFramebuffer);
	__VK_DEVICE_PROC__(DestroyFramebuffer);
	__VK_DEVICE_PROC__(CreateRenderPass);
	__VK_DEVICE_PROC__(DestroyRenderPass);
	__VK_DEVICE_PROC__(GetRenderAreaGranularity);
	__VK_DEVICE_PROC__(CreateCommandPool);
	__VK_DEVICE_PROC__(DestroyCommandPool);
	__VK_DEVICE_PROC__(ResetCommandPool);
	__VK_DEVICE_PROC__(AllocateCommandBuffers);
	__VK_DEVICE_PROC__(FreeCommandBuffers);
	__VK_DEVICE_PROC__(BeginCommandBuffer);
	__VK_DEVICE_PROC__(EndCommandBuffer);
	__VK_DEVICE_PROC__(ResetCommandBuffer);
	__VK_DEVICE_PROC__(CmdBindPipeline);
	__VK_DEVICE_PROC__(CmdSetViewport);
	__VK_DEVICE_PROC__(CmdSetScissor);
	__VK_DEVICE_PROC__(CmdSetLineWidth);
	__VK_DEVICE_PROC__(CmdSetDepthBias);
	__VK_DEVICE_PROC__(CmdSetBlendConstants);
	__VK_DEVICE_PROC__(CmdSetDepthBounds);
	__VK_DEVICE_PROC__(CmdSetStencilCompareMask);
	__VK_DEVICE_PROC__(CmdSetStencilWriteMask);
	__VK_DEVICE_PROC__(CmdSetStencilReference);
	__VK_DEVICE_PROC__(CmdBindDescriptorSets);
	__VK_DEVICE_PROC__(CmdBindIndexBuffer);
	__VK_DEVICE_PROC__(CmdBindVertexBuffers);
	__VK_DEVICE_PROC__(CmdDraw);
	__VK_DEVICE_PROC__(CmdDrawIndexed);
	__VK_DEVICE_PROC__(CmdDrawIndirect);
	__VK_DEVICE_PROC__(CmdDrawIndexedIndirect);
	__VK_DEVICE_PROC__(CmdDispatch);
	__VK_DEVICE_PROC__(CmdDispatchIndirect);
	__VK_DEVICE_PROC__(CmdCopyBuffer);
	__VK_DEVICE_PROC__(CmdCopyImage);
	__VK_DEVICE_PROC__(CmdBlitImage);
	__VK_DEVICE_PROC__(CmdCopyBufferToImage);
	__VK_DEVICE_PROC__(CmdCopyImageToBuffer);
	__VK_DEVICE_PROC__(CmdUpdateBuffer);
	__VK_DEVICE_PROC__(CmdFillBuffer);
	__VK_DEVICE_PROC__(CmdClearColorImage);
	__VK_DEVICE_PROC__(CmdClearDepthStencilImage);
	__VK_DEVICE_PROC__(CmdClearAttachments);
	__VK_DEVICE_PROC__(CmdResolveImage);
	__VK_DEVICE_PROC__(CmdSetEvent);
	__VK_DEVICE_PROC__(CmdResetEvent);
	__VK_DEVICE_PROC__(CmdWaitEvents);
	__VK_DEVICE_PROC__(CmdPipelineBarrier);
	__VK_DEVICE_PROC__(CmdBeginQuery);
	__VK_DEVICE_PROC__(CmdEndQuery);
	__VK_DEVICE_PROC__(CmdResetQueryPool);
	__VK_DEVICE_PROC__(CmdWriteTimestamp);
	__VK_DEVICE_PROC__(CmdCopyQueryPoolResults);
	__VK_DEVICE_PROC__(CmdPushConstants);
	__VK_DEVICE_PROC__(CmdBeginRenderPass);
	__VK_DEVICE_PROC__(CmdNextSubpass);
	__VK_DEVICE_PROC__(CmdEndRenderPass);
	__VK_DEVICE_PROC__(CmdExecuteCommands);
	__VK_DEVICE_PROC__(AcquireNextImageKHR);	
	__VK_DEVICE_PROC__(CreateSwapchainKHR);
	__VK_DEVICE_PROC__(DestroySwapchainKHR);
	__VK_DEVICE_PROC__(GetSwapchainImagesKHR);

	VkDevice								m_LogicalDevice;

private:
	friend class Instance;
	friend class Device;
	friend class DeviceAdapter;
	
	Gpu(VkPhysicalDevice const&, Instance* inst);

	void QuerySupportQueues();
	void LoadDeviceProcs();

	Instance*								m_Inst;
	VkPhysicalDevice						m_PhysicalGpu;
	VkPhysicalDeviceProperties				m_Prop;
	VkPhysicalDeviceMemoryProperties		m_MemProp;
	uint32									m_GraphicsQueueIndex = 0;
	uint32									m_ComputeQueueIndex = 0;
	uint32									m_CopyQueueIndex = 0;
	DynArray<VkQueueFamilyProperties>		m_QueueProps;

	PFN_vkDestroyDevice						fpDestroyDevice = NULL;
	PFN_vkFreeCommandBuffers				fpFreeCommandBuffers = NULL;
	PFN_vkCreateCommandPool					fpCreateCommandPool = NULL;
	PFN_vkAllocateCommandBuffers			fpAllocateCommandBuffers = NULL;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object, size_t location, int32_t messageCode,
	const char * pLayerPrefix,
	const char * pMessage, void * pUserData);

#define __VK_GLOBAL_LEVEL_PROC__(name)		PFN_vk##name	gp##name = NULL;
#define __VK_INSTANCE_LEVEL_PROC__(name)	PFN_vk##name	fp##name = NULL

class Instance
{
public:
	Instance(const ::k3d::String& engineName, const ::k3d::String& appName, bool enableValidation = true);
	~Instance();

	uint32	GetHostGpuCount() const { return m_Gpus.Count(); }
	GpuRef	GetHostGpuByIndex(uint32 i) const { return m_Gpus[i]; }
	rhi::DeviceRef GetDeviceByIndex(uint32 i) const { return m_LogicDevices[i]; }

	void	SetupDebugging(VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callBack);
	void	FreeDebugCallback();

	bool	WithValidation() const { return m_EnableValidation; }
	void	AppendLogicalDevice(rhi::DeviceRef logicalDevice);

	VkResult CreateSurfaceKHR(const
#if K3DPLATFORM_OS_WIN
							  VkWin32SurfaceCreateInfoKHR
#elif K3DPLATFORM_OS_ANDROID
							  VkAndroidSurfaceCreateInfoKHR
#endif
							  * pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
	{
#if K3DPLATFORM_OS_WIN
		return fpCreateWin32SurfaceKHR(m_Instance, pCreateInfo, pAllocator, pSurface);
#elif K3DPLATFORM_OS_ANDROID
		return fpCreateAndroidSurfaceKHR(m_Instance, pCreateInfo, pAllocator, pSurface);
#endif
	}

	void DestroySurfaceKHR(VkSurfaceKHR surface, VkAllocationCallbacks* pAllocator)
	{
		fpDestroySurfaceKHR(m_Instance, surface, pAllocator);
	}

	friend class Gpu;
	friend class Device;
	friend struct RHIRoot;
private:
	void LoadGlobalProcs();
	void EnumExtsAndLayers();
	void ExtractEnabledExtsAndLayers();

	void EnumGpus();
	void LoadInstanceProcs();

	bool							m_EnableValidation;
	::k3d::DynArray<::k3d::String>	m_EnabledExts;
	::k3d::DynArray<char*>			m_EnabledExtsRaw;
	::k3d::DynArray<::k3d::String>	m_EnabledLayers;
	::k3d::DynArray<const char*>	m_EnabledLayersRaw;

	VkInstance								m_Instance;
	VkDebugReportCallbackEXT				m_DebugMsgCallback;
	::k3d::DynArray<GpuRef>					m_Gpus;
	::k3d::DynArray<rhi::DeviceAdapterRef>	m_GpuAdapters;
	::k3d::DynArray<rhi::DeviceRef>			m_LogicDevices;

	//private functions 

	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	fpGetPhysicalDeviceSurfacePresentModesKHR;


	__VK_GLOBAL_LEVEL_PROC__(CreateInstance);
	__VK_GLOBAL_LEVEL_PROC__(EnumerateInstanceExtensionProperties);
	__VK_GLOBAL_LEVEL_PROC__(EnumerateInstanceLayerProperties);
	__VK_GLOBAL_LEVEL_PROC__(GetInstanceProcAddr);

	__VK_INSTANCE_LEVEL_PROC__(CreateDebugReportCallbackEXT);
	__VK_INSTANCE_LEVEL_PROC__(DestroyDebugReportCallbackEXT);

#if K3DPLATFORM_OS_WIN
	__VK_INSTANCE_LEVEL_PROC__(CreateWin32SurfaceKHR);
#elif K3DPLATFORM_OS_ANDROID
	__VK_INSTANCE_LEVEL_PROC__(CreateAndroidSurfaceKHR);
#endif
	__VK_INSTANCE_LEVEL_PROC__(DestroySurfaceKHR);

	__VK_INSTANCE_LEVEL_PROC__(EnumeratePhysicalDevices);
	__VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceProperties);
	__VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceFeatures);
	__VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceMemoryProperties);
	__VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceQueueFamilyProperties);
	__VK_INSTANCE_LEVEL_PROC__(GetPhysicalDeviceFormatProperties);
	__VK_INSTANCE_LEVEL_PROC__(CreateDevice);
	__VK_INSTANCE_LEVEL_PROC__(GetDeviceProcAddr);
	__VK_INSTANCE_LEVEL_PROC__(DestroyInstance);

	dynlib::LibRef	m_VulkanLib;
};

using InstanceRef = SharedPtr<Instance>;

K3D_VK_END

#endif