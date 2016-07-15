#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "VkEnums.h"

K3D_VK_BEGIN
/*
enum class EBindType
{
EUndefined		= 0,
EBlock			= 0x1,
ESampler		= 0x1 << 1,
EStorageImage	= 0x1 << 2,
EStorageBuffer	= 0x1 << 3,
EConstants		= 0x000000010
};
*/
VkDescriptorType RHIDataType2VkType(shaderbinding::EBindType const & type)
{
	switch (type)
	{
	case shaderbinding::EBindType::EBlock:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case shaderbinding::EBindType::ESampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case shaderbinding::EBindType::EStorageImage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case shaderbinding::EBindType::EStorageBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkDescriptorSetLayoutBinding RHIBinding2VkBinding(shaderbinding::Binding const & binding)
{
	VkDescriptorSetLayoutBinding vkbinding = {};
	vkbinding.descriptorType = RHIDataType2VkType(binding.VarType);
	vkbinding.binding = binding.VarNumber;
	vkbinding.stageFlags = g_ShaderType[binding.VarStage];
	vkbinding.descriptorCount = 1;
	return vkbinding;
}

DescriptorAllocator::DescriptorAllocator(
	Device::Ptr pDevice,
	DescriptorAllocator::Options const &option, 
	uint32 maxSets, 
	BindingArray const& bindings)
	: DeviceChild(pDevice)
	, m_Options(option)
	, m_Pool(VK_NULL_HANDLE)
{
	Initialize(maxSets, bindings);
}

DescriptorAllocator::~DescriptorAllocator()
{
	Destroy();
}

void DescriptorAllocator::Initialize(uint32 maxSets, BindingArray const& bindings)
{
	std::map<VkDescriptorType, uint32_t> mappedCounts;
	for (const auto& binding : bindings) 
	{
		VkDescriptorType descType = binding.descriptorType;
		auto it = mappedCounts.find(descType);
		if (it != mappedCounts.end()) 
		{
			++mappedCounts[descType];
		}
		else
		{
			mappedCounts[descType] = 1;
		}
	}

	std::vector<VkDescriptorPoolSize> typeCounts;
	for (const auto &mc : mappedCounts)
	{
		typeCounts.push_back({ mc.first, mc.second });
	}

	VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	createInfo.flags = m_Options.CreateFlags;
	createInfo.maxSets = 1;
	createInfo.poolSizeCount = typeCounts.size();
	createInfo.pPoolSizes = typeCounts.size()==0 ? nullptr : typeCounts.data();
	K3D_VK_VERIFY(vkCreateDescriptorPool(GetRawDevice(), &createInfo, NULL, &m_Pool));
}

void DescriptorAllocator::Destroy()
{
	if (VK_NULL_HANDLE == m_Pool)
		return;
	vkDestroyDescriptorPool(GetRawDevice(), m_Pool, nullptr);
	m_Pool = VK_NULL_HANDLE;
	VKLOG(Info, "DescriptorAllocator-destroying vkDestroyDescriptorPool...");
}

DescriptorSetLayout::DescriptorSetLayout(Device::Ptr pDevice, BindingArray const & bindings)
: DeviceChild(pDevice)
, m_DescriptorSetLayout(VK_NULL_HANDLE)
{
	Initialize(bindings);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	Destroy();
}

void DescriptorSetLayout::Initialize(BindingArray const & bindings)
{
	VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    createInfo.bindingCount	= bindings.Count();
    createInfo.pBindings	= bindings.Count()==0 ? nullptr : bindings.Data();
    K3D_VK_VERIFY(vkCmd::CreateDescriptorSetLayout( GetRawDevice(), &createInfo, nullptr, &m_DescriptorSetLayout ));
}

void DescriptorSetLayout::Destroy()
{
	if( VK_NULL_HANDLE == m_DescriptorSetLayout ) 
		return;
	vkDestroyDescriptorSetLayout( GetRawDevice(), m_DescriptorSetLayout, nullptr );
	m_DescriptorSetLayout = VK_NULL_HANDLE;
	VKLOG(Info, "DescriptorSetLayout-destroying vkDescriptorSetLayout...");
}

DescriptorSet::DescriptorSet( DescriptorAllocator *descriptorAllocator, VkDescriptorSetLayout layout, BindingArray const & bindings, Device::Ptr pDevice )
	: DeviceChild( pDevice )
	, m_DescriptorAllocator( descriptorAllocator )
	, m_Bindings(bindings)
{
	Initialize( layout, bindings );
}


DescriptorSet * DescriptorSet::CreateDescSet(DescriptorAllocator * descriptorPool, VkDescriptorSetLayout layout, BindingArray const & bindings, Device::Ptr pDevice)
{
	return new DescriptorSet(descriptorPool, layout, bindings, pDevice);
}

DescriptorSet::~DescriptorSet()
{
	Destroy();
}

void DescriptorSet::Update(uint32 bindSet, rhi::IGpuResource * gpuResource)
{
	switch(gpuResource->GetResourceType())
	{
	case rhi::EGT_Buffer:
		VkDescriptorBufferInfo bufferInfo = { (VkBuffer)gpuResource->GetResourceLocation(), 0, gpuResource->GetResourceSize() };
		m_BoundDescriptorSet[bindSet].pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(GetRawDevice(), 1, &m_BoundDescriptorSet[bindSet], 0, NULL);
		VKLOG(Info, "%s , Set (0x%0x) updated with buffer(location:0x%x, size:%d).", __K3D_FUNC__, m_DescriptorSet, 
			gpuResource->GetResourceLocation(), gpuResource->GetResourceSize());
		break;
	}
}

void DescriptorSet::Initialize( VkDescriptorSetLayout layout, BindingArray const & bindings)
{
	std::vector<VkDescriptorSetLayout> layouts = { layout };
	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool		= m_DescriptorAllocator->m_Pool;
	allocInfo.descriptorSetCount	= static_cast<uint32_t>( layouts.size() );
	allocInfo.pSetLayouts			= layouts.empty() ? nullptr : layouts.data();
	K3D_VK_VERIFY( vkAllocateDescriptorSets( GetRawDevice(), &allocInfo, &m_DescriptorSet ) );
	VKLOG(Info, "%s , Set (0x%0x) created.", __K3D_FUNC__, m_DescriptorSet);

	for (auto& binding : m_Bindings)
	{
		VkWriteDescriptorSet entry = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		entry.dstSet = m_DescriptorSet;
		entry.descriptorCount = 1;
		entry.dstArrayElement = 0;
		entry.dstBinding = binding.binding;
		entry.descriptorType = binding.descriptorType;
		m_BoundDescriptorSet.push_back(entry);
	}
}

void DescriptorSet::Destroy()
{
	if( VK_NULL_HANDLE == m_DescriptorSet ) 
		return;

	if( nullptr != m_DescriptorAllocator )
	{
		//const auto& options = m_DescriptorAllocator->m_Options;
		//if( options.hasFreeDescriptorSetFlag() ) {
			VkDescriptorSet descSets[1] = { m_DescriptorSet };
			vkFreeDescriptorSets( GetRawDevice(), m_DescriptorAllocator->m_Pool, 1, descSets );
		//}
	}
	m_DescriptorSet = VK_NULL_HANDLE;
	m_DescriptorAllocator = nullptr;
	VKRHI_METHOD_TRACE
}

K3D_VK_END