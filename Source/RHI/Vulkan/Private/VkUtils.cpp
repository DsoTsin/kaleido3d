#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkUtils.h"

K3D_VK_BEGIN

DescriptorAllocator::DescriptorAllocator(Device::Ptr pDevice)
	: DeviceChild(pDevice)
{
	K3D_VK_VERIFY(vkCreateDescriptorPool(GetRawDevice(), /*&descPoolCreateInfo*/nullptr, NULL, &m_Pool));
}

DescriptorAllocator::~DescriptorAllocator()
{
	vkDestroyDescriptorPool(GetRawDevice(), m_Pool, nullptr);
	Log::Out(LogLevel::Info, "DescriptorAllocator", "destroying vkDestroyDescriptorPool...");
}

VkDeviceSize calcAlignedOffset(VkDeviceSize offset, VkDeviceSize align)
{
	VkDeviceSize n = offset / align;
	VkDeviceSize r = offset % align;
	VkDeviceSize result = (n + (r > 0 ? 1 : 0)) * align;
	return result;
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

FrameBuffer::FrameBuffer(Device::Ptr pDevice, VkRenderPass renderPass, FrameBuffer::Option const& op)
	: DeviceChild(pDevice)
	, m_RenderPass(renderPass)
	, m_Width(op.Width)
	, m_Height(op.Height)
{
	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.renderPass = m_RenderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(op.Attachments.size());
	createInfo.pAttachments = op.Attachments.data();
	createInfo.width = m_Width;
	createInfo.height = m_Height;
	createInfo.layers = 1;
	createInfo.flags = 0;
	K3D_VK_VERIFY(vkCreateFramebuffer(GetRawDevice(), &createInfo, nullptr, &m_FrameBuffer));
}

FrameBuffer::~FrameBuffer()
{
	if (VK_NULL_HANDLE == m_FrameBuffer)
	{
		return;
	}
	vkDestroyFramebuffer(GetRawDevice(), m_FrameBuffer, nullptr);
	m_FrameBuffer = VK_NULL_HANDLE;
}

RenderPass::RenderPass(Device::Ptr pDevice, RenderpassOptions const & options)
	: DeviceChild(pDevice)
{
	Initialize(options);
}

RenderPass::~RenderPass()
{
	Destroy();
}

void RenderPass::BeginRender(const PtrContext & context, const PtrFrameBuffer & framebuffer)
{
	m_GfxContext = context;
	m_FrameBuffer = framebuffer;

	mSubpass = 0;
	//vk::context()->pushRenderPass(this->shared_from_this());
	//vk::context()->pushSubPass(mSubpass);

	// Start the command buffer
	context->Begin();
	//vk::context()->pushCommandBuffer(mCommandBuffer);

	// Tranmsfer uniform data
	// vk::context()->transferPendingUniformBuffer(mCommandBuffer, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);


	rhi::ViewportDesc desc{ 1.0f*m_FrameBuffer->GetWidth(), 1.0f*m_FrameBuffer->GetHeight() };
	context->SetViewport(desc);
	VkRect2D ra = {};
	ra.offset = {0,0};
	ra.extent = { m_FrameBuffer->GetWidth(), m_FrameBuffer->GetHeight() };
	context->SetScissorRects(1, &ra);

	//// Begin the render pass
	//const auto& clearValues = getAttachmentClearValues();
	VkRenderPassBeginInfo renderPassBegin;
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.pNext = NULL;
	renderPassBegin.renderPass = m_RenderPass;
	renderPassBegin.framebuffer = m_FrameBuffer->m_FrameBuffer;
	renderPassBegin.renderArea = ra;
	//renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	//renderPassBegin.pClearValues = clearValues.empty() ? nullptr : clearValues.data();
	context->BeginRenderPass(&renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::EndRender()
{
	// End render pass
	m_GfxContext->EndRenderPass();

	// End the command buffer
	m_GfxContext->End();
	//vk::context()->popCommandBuffer();

	//// Process the command buffer
	m_GfxContext->SubmitAndWait(nullptr, nullptr, nullptr);
	//vk::context()->getGraphicsQueue()->waitIdle();

	//// Pop them
	//vk::context()->popSubPass();
	//vk::context()->popRenderPass();

	//// Clear these vars
	//mFramebuffer.reset();
	//mCommandBuffer.reset();
}

void RenderPass::BeginRenderExplicit(const VkCommandBuffer & cmdBuf, const PtrFrameBuffer & framebuffer)
{
}

void RenderPass::EndRenderExplicit()
{
}

void RenderPass::NextSubpass()
{
	m_GfxContext->NextSubpass(VK_SUBPASS_CONTENTS_INLINE);
	++mSubpass;
	//vk::context()->setSubpass(mSubpass);
}

void RenderPass::Initialize(RenderpassOptions const & options)
{
	if (VK_NULL_HANDLE != m_RenderPass)
	{
		return;
	}

	m_Options = options;

	K3D_ASSERT(options.m_Attachments.size() > 0);
	K3D_ASSERT(options.m_Subpasses.size() > 0);

	// Populate attachment descriptors
	const size_t numAttachmentDesc = options.m_Attachments.size();
	mAttachmentDescriptors.resize(numAttachmentDesc);
	mAttachmentClearValues.resize(numAttachmentDesc);
	for (size_t i = 0; i < numAttachmentDesc; ++i) {
		mAttachmentDescriptors[i] = options.m_Attachments[i].GetDescription();
		mAttachmentClearValues[i] = options.m_Attachments[i].GetClearValue();
	}

	// Populate attachment references
	const size_t numSubPasses = options.m_Subpasses.size();
	std::vector<Subpass::AttachReferences> subPassAttachmentRefs(numSubPasses);
	for (size_t i = 0; i < numSubPasses; ++i) {
		const auto& subPass = options.m_Subpasses[i];

		// Color attachments
		{
			// Allocate elements for color attachments
			const size_t numColorAttachments = subPass.m_ColorAttachments.size();
			subPassAttachmentRefs[i].Color.resize(numColorAttachments);
			subPassAttachmentRefs[i].Resolve.resize(numColorAttachments);

			// Populate color and resolve attachments
			for (size_t j = 0; j < numColorAttachments; ++j) {
				// color
				uint32_t colorAttachmentIndex = subPass.m_ColorAttachments[j];
				VkImageLayout colorImageLayout = mAttachmentDescriptors[colorAttachmentIndex].initialLayout;;
				subPassAttachmentRefs[i].Color[j] = {};
				subPassAttachmentRefs[i].Color[j].attachment = colorAttachmentIndex;
				subPassAttachmentRefs[i].Color[j].layout = colorImageLayout;
				// resolve
				uint32_t resolveAttachmentIndex = subPass.m_ResolveAttachments[j];
				subPassAttachmentRefs[i].Resolve[j] = {};
				subPassAttachmentRefs[i].Resolve[j].attachment = resolveAttachmentIndex;
				subPassAttachmentRefs[i].Resolve[j].layout = colorImageLayout; // Not a mistake, this is on purpose
			}
		}

		// Depth/stencil attachment
		std::vector<VkAttachmentReference> depthStencilAttachmentRef;
		if (!subPass.m_DepthStencilAttachment.empty()) {
			// Allocate elements for depth/stencil attachments
			subPassAttachmentRefs[i].Depth.resize(1);

			// Populate depth/stencil attachments
			uint32_t attachmentIndex = subPass.m_DepthStencilAttachment[0];
			subPassAttachmentRefs[i].Depth[0] = {};
			subPassAttachmentRefs[i].Depth[0].attachment = attachmentIndex;
			subPassAttachmentRefs[i].Depth[0].layout = mAttachmentDescriptors[attachmentIndex].initialLayout;
		}

		// Preserve attachments
		if (!subPass.m_PreserveAttachments.empty()) {
			subPassAttachmentRefs[i].Preserve = subPass.m_PreserveAttachments;
		}
	}

	// Populate sub passes
	std::vector<VkSubpassDescription> subPassDescs(numSubPasses);
	for (size_t i = 0; i < numSubPasses; ++i) {
		const auto& colorAttachmentRefs = subPassAttachmentRefs[i].Color;
		const auto& resolveAttachmentRefs = subPassAttachmentRefs[i].Resolve;
		const auto& depthStencilAttachmentRef = subPassAttachmentRefs[i].Depth;
		const auto& preserveAttachmentRefs = subPassAttachmentRefs[i].Preserve;

		bool noResolves = true;
		for (const auto& attachRef : resolveAttachmentRefs) {
			if (VK_ATTACHMENT_UNUSED != attachRef.attachment) {
				noResolves = false;
				break;
			}
		}

		subPassDescs[i] = {};
		auto& desc = subPassDescs[i];
		desc.pipelineBindPoint = options.m_Subpasses[i].m_PipelineBindPoint;
		desc.flags = 0;
		desc.inputAttachmentCount = 0;
		desc.pInputAttachments = nullptr;
		desc.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
		desc.pColorAttachments = colorAttachmentRefs.empty() ? nullptr : colorAttachmentRefs.data();
		desc.pResolveAttachments = (resolveAttachmentRefs.empty() || noResolves) ? nullptr : resolveAttachmentRefs.data();
		desc.pDepthStencilAttachment = depthStencilAttachmentRef.empty() ? nullptr : depthStencilAttachmentRef.data();
		desc.preserveAttachmentCount = static_cast<uint32_t>(preserveAttachmentRefs.size());
		desc.pPreserveAttachments = preserveAttachmentRefs.empty() ? nullptr : preserveAttachmentRefs.data();
	}

	// Cache the subpass sample counts
	for (auto& subpass : m_Options.m_Subpasses) {
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		// Look at color attachments first..
		if (!subpass.m_ColorAttachments.empty()) {
			uint32_t attachmentIndex = subpass.m_ColorAttachments[0];
			sampleCount = m_Options.m_Attachments[attachmentIndex].m_Description.samples;
		}
		// ..and then look at depth attachments
		if ((VK_SAMPLE_COUNT_1_BIT == sampleCount) && (!subpass.m_DepthStencilAttachment.empty())) {
			uint32_t attachmentIndex = subpass.m_DepthStencilAttachment[0];
			sampleCount = m_Options.m_Attachments[attachmentIndex].m_Description.samples;
		}
		// Cache it
		mSubpassSampleCounts.push_back(sampleCount);
	}

	std::vector<VkSubpassDependency> dependencies;
	for (auto& subpassDep : m_Options.m_SubpassDependencies) {
		dependencies.push_back(subpassDep.m_Dependency);
	}

	// Create render pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(mAttachmentDescriptors.size());
	renderPassCreateInfo.pAttachments = &(mAttachmentDescriptors[0]);
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subPassDescs.size());
	renderPassCreateInfo.pSubpasses = subPassDescs.empty() ? nullptr : subPassDescs.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassCreateInfo.pDependencies = dependencies.empty() ? nullptr : dependencies.data();
	K3D_VK_VERIFY(vkCreateRenderPass(GetRawDevice(), &renderPassCreateInfo, nullptr, &m_RenderPass));
}

void RenderPass::Destroy()
{
	if (VK_NULL_HANDLE == m_RenderPass)
	{
		return;
	}
	vkDestroyRenderPass(GetRawDevice(), m_RenderPass, nullptr);
	m_RenderPass = VK_NULL_HANDLE;
}

K3D_VK_END