#ifndef __TextureObject_h__
#define __TextureObject_h__

#include <Interface/IRHI.h>
/*
* Defines a simple object for creating and holding Vulkan texture objects.
* Supports loading from TGA files in Android Studio asset folder.
*
* Only supports R8G8B8A8 files and texture formats. Converts from TGA BGRA to RGBA.
*/
class TextureObject
{
public:
	TextureObject(rhi::DeviceRef pDevice, const uint8_t* dataInMemory, bool useStaging = true);
	~TextureObject();

	uint64 GetSize() const { return m_DataSize; }

	void MapIntoBuffer(rhi::GpuResourceRef stageBuff);

	void CopyAndInitTexture(rhi::GpuResourceRef stageBuff);

	rhi::GpuResourceRef GetResource() const { return m_Resource; }

	rhi::SamplerRef GetSampler()
	{
		return m_sampler;
	}

protected:
	void InitData(const uint8* dataInMemory);
	void InitTexture();
	void InitSampler();
	bool Destroy();

private:
	rhi::DeviceRef m_pDevice;
	rhi::CommandContextRef m_pContext;
	rhi::GpuResourceRef m_Resource;
	rhi::SamplerRef m_sampler;
	rhi::EPixelFormat m_format;
	bool	m_UseStaging;
	uint64	m_DataSize;
	uint32_t m_width;
	uint32_t m_height;
	uint8*   m_pBits;
};

#endif