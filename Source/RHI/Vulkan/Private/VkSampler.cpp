#include "VkCommon.h"
#include "VkRHI.h"
#include "VkEnums.h"

K3D_VK_BEGIN

Sampler::Sampler(Device::Ptr pDevice, rhi::SamplerState const & samplerDesc)
	: DeviceChild(pDevice)
	, m_SamplerState(samplerDesc)
{
	if (pDevice)
	{
		m_SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		m_SamplerCreateInfo.magFilter = g_Filters[samplerDesc.Filter.MagFilter];
		m_SamplerCreateInfo.minFilter = g_Filters[samplerDesc.Filter.MinFilter];
		m_SamplerCreateInfo.mipmapMode = g_MipMapModes[samplerDesc.Filter.MipMapFilter];
		m_SamplerCreateInfo.addressModeU = g_AddressModes[samplerDesc.U];
		m_SamplerCreateInfo.addressModeV = g_AddressModes[samplerDesc.V];
		m_SamplerCreateInfo.addressModeW = g_AddressModes[samplerDesc.W];
		m_SamplerCreateInfo.mipLodBias = samplerDesc.MipLODBias;
		m_SamplerCreateInfo.compareOp = g_ComparisonFunc[samplerDesc.ComparisonFunc];
		m_SamplerCreateInfo.minLod = samplerDesc.MinLOD;
		// Max level-of-detail should match mip level count
		m_SamplerCreateInfo.maxLod = samplerDesc.MaxLOD;
		// Enable anisotropic filtering
		m_SamplerCreateInfo.maxAnisotropy = samplerDesc.MaxAnistropy;
		m_SamplerCreateInfo.anisotropyEnable = VK_TRUE;
		m_SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; // cannot convert...
		GetGpuRef()->vkCreateSampler(GetRawDevice(), &m_SamplerCreateInfo, nullptr, &m_Sampler);
	}
}

Sampler::~Sampler()
{
	VKLOG(Info, "Sampler-Destroying ...");
	GetGpuRef()->vkDestroySampler(GetRawDevice(), m_Sampler, nullptr);
}

rhi::SamplerState Sampler::GetSamplerDesc() const
{
	return m_SamplerState;
}

K3D_VK_END