#ifndef __VkObjects_h__
#define __VkObjects_h__
#pragma once

K3D_VK_BEGIN


struct K3D_API AttachmentDescription : public VkAttachmentDescription
{
	AttachmentDescription(VkFormat format, VkSampleCountFlagBits samples)
	{
		format = format;
		samples = samples;
		loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	AttachmentDescription() {}
	AttachmentDescription & Format(VkFormat const & fmt) { this->format = fmt; return *this; }
	AttachmentDescription & Samples(VkSampleCountFlagBits const & sample) { this->samples = sample; return *this; }
	AttachmentDescription & LoadOp(VkAttachmentLoadOp const & op) { this->loadOp = op; return *this; }
	AttachmentDescription & StoreOp(VkAttachmentStoreOp const & op) { this->storeOp = op; return *this; }
	AttachmentDescription & StencilLoadOp(VkAttachmentLoadOp const & op) { this->stencilLoadOp = op; return *this; }
	AttachmentDescription & StencilStoreOp(VkAttachmentStoreOp const & op) { this->stencilStoreOp = op; return *this; }
	AttachmentDescription & InitialLayout(VkImageLayout const & layout) { this->initialLayout = layout; return *this; }
	AttachmentDescription & FinalLayout(VkImageLayout const & layout) { this->finalLayout = layout; return *this; }
};

class K3D_API RenderpassAttachment
{
public:
	RenderpassAttachment(VkFormat fmt = VK_FORMAT_UNDEFINED, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	virtual ~RenderpassAttachment() {}

	RenderpassAttachment&			SetClearValue(const VkClearValue& value) { m_ClearValue = value; return *this; }
	const VkClearValue&				GetClearValue() const { return m_ClearValue; }
	const VkAttachmentDescription&	GetDescription() const { return m_Description; }
	AttachmentDescription&			Description() { return m_Description; }

	static RenderpassAttachment	CreateColor(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	static RenderpassAttachment	CreateDepthStencil(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

	friend class			RenderPass;
private:
	AttachmentDescription	m_Description;
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

class K3D_API RenderpassOptions
{
public:
	RenderpassOptions() {}
	RenderpassOptions(VkFormat colorFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	RenderpassOptions(VkFormat colorFormat, VkFormat depthStencilFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	virtual ~RenderpassOptions() {}

	RenderpassOptions&				AddAttachment(const RenderpassAttachment& value) { m_Attachments.push_back(value); return *this; }
	RenderpassOptions&				AddSubPass(const Subpass& value) { m_Subpasses.push_back(value); return *this; }
	RenderpassOptions&				AddSubpassDependency(const SubpassDependency& value) { m_SubpassDependencies.push_back(value); return *this; }

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
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			nullptr,
			0
		};
	}

};

K3D_VK_END

#endif