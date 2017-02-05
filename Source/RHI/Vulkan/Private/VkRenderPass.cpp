#include "VkCommon.h"
#include "VkRHI.h"
#include "VkUtils.h"

K3D_VK_BEGIN

RenderPass::RenderPass(Device::Ptr pDevice, RenderpassOptions const & options)
	: DeviceChild(pDevice)
	, m_Options(options)
{
	Initialize(options);
}

RenderPass::RenderPass(Device::Ptr pDevice, RenderTargetLayout const & rtl)
	: DeviceChild(pDevice)
{
	Initialize(rtl);
}

RenderPass::~RenderPass()
{
	Destroy();
}

void RenderPass::NextSubpass()
{
	m_GfxContext->NextSubpass(VK_SUBPASS_CONTENTS_INLINE);
	++mSubpass;
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
	for (size_t i = 0; i < numAttachmentDesc; ++i)
	{
		mAttachmentDescriptors[i] = options.m_Attachments[i].GetDescription();
		mAttachmentClearValues[i] = options.m_Attachments[i].GetClearValue();
	}

	// Populate attachment references
	const size_t numSubPasses = options.m_Subpasses.size();
	std::vector<Subpass::AttachReferences> subPassAttachmentRefs(numSubPasses);
	for (size_t i = 0; i < numSubPasses; ++i)
	{
		const auto& subPass = options.m_Subpasses[i];

		// Color attachments
		{
			// Allocate elements for color attachments
			const size_t numColorAttachments = subPass.m_ColorAttachments.size();
			subPassAttachmentRefs[i].Color.resize(numColorAttachments);
			subPassAttachmentRefs[i].Resolve.resize(numColorAttachments);

			// Populate color and resolve attachments
			for (size_t j = 0; j < numColorAttachments; ++j)
			{
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
		if (!subPass.m_DepthStencilAttachment.empty())
		{
			// Allocate elements for depth/stencil attachments
			subPassAttachmentRefs[i].Depth.resize(1);

			// Populate depth/stencil attachments
			uint32_t attachmentIndex = subPass.m_DepthStencilAttachment[0];
			subPassAttachmentRefs[i].Depth[0] = {};
			subPassAttachmentRefs[i].Depth[0].attachment = attachmentIndex;
			subPassAttachmentRefs[i].Depth[0].layout = mAttachmentDescriptors[attachmentIndex].initialLayout;
		}

		// Preserve attachments
		if (!subPass.m_PreserveAttachments.empty())
		{
			subPassAttachmentRefs[i].Preserve = subPass.m_PreserveAttachments;
		}
	}

	// Populate sub passes
	std::vector<VkSubpassDescription> subPassDescs(numSubPasses);
	for (size_t i = 0; i < numSubPasses; ++i)
	{
		const auto& colorAttachmentRefs = subPassAttachmentRefs[i].Color;
		const auto& resolveAttachmentRefs = subPassAttachmentRefs[i].Resolve;
		const auto& depthStencilAttachmentRef = subPassAttachmentRefs[i].Depth;
		const auto& preserveAttachmentRefs = subPassAttachmentRefs[i].Preserve;

		bool noResolves = true;
		for (const auto& attachRef : resolveAttachmentRefs)
		{
			if (VK_ATTACHMENT_UNUSED != attachRef.attachment)
			{
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
	for (auto& subpass : m_Options.m_Subpasses)
	{
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		// Look at color attachments first..
		if (!subpass.m_ColorAttachments.empty())
		{
			uint32_t attachmentIndex = subpass.m_ColorAttachments[0];
			sampleCount = m_Options.m_Attachments[attachmentIndex].m_Description.samples;
		}
		// ..and then look at depth attachments
		if ((VK_SAMPLE_COUNT_1_BIT == sampleCount) && (!subpass.m_DepthStencilAttachment.empty()))
		{
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
	renderPassCreateInfo.pAttachments = mAttachmentDescriptors.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subPassDescs.size());
	renderPassCreateInfo.pSubpasses = subPassDescs.empty() ? nullptr : subPassDescs.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassCreateInfo.pDependencies = dependencies.empty() ? nullptr : dependencies.data();
	K3D_VK_VERIFY(GetGpuRef()->vkCreateRenderPass(GetRawDevice(), &renderPassCreateInfo, nullptr, &m_RenderPass));
}

void RenderPass::Initialize(RenderTargetLayout const & rtl)
{
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = rtl.GetNumColorAttachments();
	subpass.pColorAttachments = rtl.GetColorAttachmentReferences();
	subpass.pResolveAttachments = rtl.GetResolveAttachmentReferences();
	subpass.pDepthStencilAttachment = rtl.GetDepthStencilAttachmentReference();
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;
	
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	
	renderPassCreateInfo.attachmentCount = rtl.GetNumAttachments();
	renderPassCreateInfo.pAttachments = rtl.GetAttachmentDescriptions();

	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;
	K3D_VK_VERIFY(GetGpuRef()->vkCreateRenderPass(GetRawDevice(), &renderPassCreateInfo, nullptr, &m_RenderPass));
}

void RenderPass::Destroy()
{
	if (VK_NULL_HANDLE == m_RenderPass)
	{
		return;
	}
	VKLOG(Info, "RenderPass destroy... -- %0x.", m_RenderPass);
	GetGpuRef()->vkDestroyRenderPass(GetRawDevice(), m_RenderPass, nullptr);
	m_RenderPass = VK_NULL_HANDLE;
}

K3D_VK_END