#include "VkCommon.h"
#include "Public/VkRHI.h"

K3D_VK_BEGIN

Resource::~Resource()
{
	Log::Out(LogLevel::Info, "Resource", "Destroying Resource..");
	vkFreeMemory(GetRawDevice(), m_DeviceMem, nullptr);
}

Buffer::~Buffer()
{
	Log::Out(LogLevel::Info, "Buffer", "Destroying Resource..");
	vkDestroyBufferView(GetRawDevice(), m_BufferView, nullptr);
	vkDestroyBuffer(GetRawDevice(), m_Buffer, nullptr);
}

Texture::~Texture()
{
	Log::Out(LogLevel::Info, "Texture", "Destroying Texture..");
	vkDestroyImageView(GetRawDevice(), m_ImageView, nullptr);
	vkDestroyImage(GetRawDevice(), m_Image, nullptr);
}

K3D_VK_END