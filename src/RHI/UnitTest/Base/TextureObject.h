#ifndef __TextureObject_h__
#define __TextureObject_h__

#include <Core/Interface/IRHI.h>
/*
* Defines a simple object for creating and holding Vulkan texture objects.
* Supports loading from TGA files in Android Studio asset folder.
*
* Only supports R8G8B8A8 files and texture formats. Converts from TGA BGRA to RGBA.
*/
class TextureObject
{
public:
	TextureObject(NGFXDeviceRef pDevice, const uint8_t* dataInMemory, bool useStaging = true);
	~TextureObject();

	uint64 GetSize() const { return m_DataSize; }

	void MapIntoBuffer(NGFXResourceRef stageBuff);

	void CopyAndInitTexture(NGFXResourceRef stageBuff);

	NGFXResourceRef GetResource() const { return m_Resource; }

	NGFXSamplerRef GetSampler()
	{
		return m_sampler;
	}

protected:
	void InitData(const uint8* dataInMemory);
	void InitTexture();
	void InitSampler();
	bool Destroy();

private:
	NGFXDeviceRef m_pDevice;
	NGFXResourceRef m_Resource;
	NGFXSamplerRef m_sampler;
	NGFXPixelFormat m_format;
	bool	m_UseStaging;
	uint64	m_DataSize;
	uint32_t m_width;
	uint32_t m_height;
	uint8*   m_pBits;
};

#endif