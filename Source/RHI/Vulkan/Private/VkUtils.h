#ifndef __VkUtils_h__
#define __VkUtils_h__

K3D_VK_BEGIN

extern K3D_API VkDeviceSize calcAlignedOffset(VkDeviceSize offset, VkDeviceSize align);

class CommandAllocator;
using PtrCmdAlloc = std::shared_ptr<CommandAllocator>;

class K3D_API CommandAllocator : public DeviceChild
{
public:
	static PtrCmdAlloc			CreateAllocator(
									uint32 queueFamilyIndex, 
									bool transient, 
									Device::Ptr device);

		~CommandAllocator();

	VkCommandPool				GetCommandPool() const { return m_Pool; }
protected:
	void						Initialize();
	void						Destroy();

private:
	CommandAllocator(uint32 queueFamilyIndex, bool transient, Device::Ptr device);

	VkCommandPool	m_Pool;
	bool			m_Transient;
	uint32			m_FamilyIndex;
};

class Texture;
class RenderPass;

class FrameBuffer : public DeviceChild
{
public:
	struct Option
	{
		uint32						Width,Height;
		std::vector<VkImageView>	Attachments;
	};

	/**
	 * create framebuffer with SwapChain ImageViews
	 */
	FrameBuffer(Device::Ptr pDevice, VkRenderPass renderPass, Option const& op);
	~FrameBuffer();

	uint32 GetWidth() const { return m_Width; }
	uint32 GetHeight() const { return m_Height; }

private:
	friend class			RenderPass;

	VkFramebuffer			m_FrameBuffer;
	VkRenderPass			m_RenderPass = VK_NULL_HANDLE;
	uint32					m_Width = 0;
	uint32					m_Height = 0;
};

using PtrFrameBuffer = std::shared_ptr<FrameBuffer>;

class RenderPass : public DeviceChild
{
public:
	typedef RenderPass	*		Ptr;

	RenderPass(Device::Ptr pDevice, RenderpassOptions const & options);
	~RenderPass();


	void				BeginRender(const PtrContext & context, const PtrFrameBuffer& framebuffer);
	void				EndRender();

	void				BeginRenderExplicit(const VkCommandBuffer& cmdBuf, const PtrFrameBuffer& framebuffer);
	void				EndRenderExplicit();

	void				NextSubpass();

private:
	RenderpassOptions					m_Options;
	PtrContext							m_GfxContext;
	PtrFrameBuffer						m_FrameBuffer;
	VkRenderPass						m_RenderPass;

	uint32								mSubpass = 0;
	std::vector<VkSampleCountFlagBits>	mSubpassSampleCounts;
	std::vector<uint32>					mBarrieredAttachmentIndices;
	std::vector<VkAttachmentDescription>mAttachmentDescriptors;
	std::vector<VkClearValue>			mAttachmentClearValues;

	void				Initialize(RenderpassOptions const & options);
	void				Destroy();

	friend class		FrameBuffer;
};

class Semaphore : public DeviceChild
{
public:
	Semaphore(Device::Ptr pDevice, VkSemaphoreCreateInfo const & info = SemaphoreCreateInfo::Create())
		: DeviceChild(pDevice)
	{
		K3D_VK_VERIFY(vkCreateSemaphore(GetRawDevice(), &info, nullptr, &m_Semaphore));
	}
	~Semaphore() { vkDestroySemaphore(GetRawDevice(), m_Semaphore, nullptr); }

private:
	friend class SwapChain;
	friend class CommandContext;

	VkSemaphore m_Semaphore;
};


K3D_VK_END

#endif // !__VkUtils_h__
