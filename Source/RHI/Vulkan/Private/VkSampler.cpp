#include "VkCommon.h"
#include "Public/VkRHI.h"
#include "Base/vulkantools.h"

K3D_VK_BEGIN

Sampler::Sampler(Device::Ptr pDevice)
	: DeviceChild(pDevice)
{
	VkSamplerCreateInfo samplerInfo = vkTools::initializers::samplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	// Max level-of-detail should match mip level count
	//samplerInfo.maxLod = (useStaging) ? (float)texture.mipLevels : 0.0f;
	// Enable anisotropic filtering
	samplerInfo.maxAnisotropy = 8;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	vkCreateSampler(GetRawDevice(), &samplerInfo, nullptr, &m_Sampler);
}

Sampler::~Sampler()
{
	Log::Out(LogLevel::Fatal, "Sampler", "Destroying ...");
	vkDestroySampler(GetRawDevice(), m_Sampler, nullptr);
}

K3D_VK_END