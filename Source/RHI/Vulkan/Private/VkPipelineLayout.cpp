#include "VkCommon.h"
#include "VkRHI.h"
#include "Private/VkUtils.h"

K3D_VK_BEGIN

PipelineLayout::PipelineLayout(Device::Ptr pDevice, rhi::PipelineLayoutDesc const & desc)
	: DeviceChild(pDevice)
	, m_PipelineLayout(VK_NULL_HANDLE)
	, m_DescSetLayout(nullptr)
	, m_DescSet()
{
	InitWithDesc(desc);
}

PipelineLayout::~PipelineLayout()
{
	Destroy();
}

void PipelineLayout::Destroy()
{
	if (m_PipelineLayout == VK_NULL_HANDLE || !GetRawDevice())
		return;
	GetGpuRef()->vkDestroyPipelineLayout(GetRawDevice(), m_PipelineLayout, nullptr);
	VKLOG(Info, "PipelineLayout Destroyed . -- %0x.", m_PipelineLayout);
	m_PipelineLayout = VK_NULL_HANDLE;
}

void PipelineLayout::InitWithDesc(rhi::PipelineLayoutDesc const & desc)
{
	DescriptorAllocator::Options options;
	BindingArray array;
	for (auto & rhibinding : desc.Bindings)
	{
		array.Append(RHIBinding2VkBinding(rhibinding));
	}

	auto alloc = GetDevice()->NewDescriptorAllocator(16, array);
	m_DescSetLayout = GetDevice()->NewDescriptorSetLayout(array);
	m_DescSet = rhi::DescriptorRef( DescriptorSet::CreateDescSet(alloc, m_DescSetLayout->GetNativeHandle(), array, GetDevice()) );

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &m_DescSetLayout->m_DescriptorSetLayout;
	K3D_VK_VERIFY(GetGpuRef()->vkCreatePipelineLayout(GetRawDevice(), &pPipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));
}

K3D_VK_END