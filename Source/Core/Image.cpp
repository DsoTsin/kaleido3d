#include "Kaleido3D.h"
#include "Image.h"

//---------------------------------------------------------------
// DDS image loader implement ??
namespace k3d {

	Image::Image()
		: m_ImgWidth(0)
		, m_ImgHeight(0)
		, m_ImgLayers(0)
		, m_MipLev(0)
	{
	}

	Image::~Image()
	{
		if (!m_ImgData.empty()) m_ImgData.clear();
	}

	uint32 Image::GetImageSize(uint32 level) const
	{
		bool compressed = IsCompressed();
		int32_t w = m_ImgWidth >> level;
		int32_t h = m_ImgHeight >> level;
		int32_t d = m_ImgDepth >> level;
		w = (w) ? w : 1;
		h = (h) ? h : 1;
		d = (d) ? d : 1;
		int32_t bw = (compressed) ? (w + 3) / 4 : w;
		int32_t bh = (compressed) ? (h + 3) / 4 : h;
		int32_t elementSize = m_ElementSize;

		return bw*bh*d*elementSize;
	}

	const void *Image::GetLevel(uint32 level, uint32 face) const
	{
		assert(level < (uint32)m_MipLev);
		//  assert( face >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
		//  assert( face == GL_TEXTURE_CUBE_MAP_POSITIVE_X || _cubeMap);
		//  face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		assert((face*m_MipLev + level) < (int32)m_ImgData.size());
		if (level >= (uint32)m_MipLev || (int32)face >= m_ImgLayers)
			return nullptr;

		return m_ImgData[face*m_MipLev + level];
	}

	bool Image::Load(uint8 *dataPtr, uint32 length)
	{
		K3D_UNUSED(dataPtr);
		K3D_UNUSED(length);
		return false;
	}

	bool Image::IsCompressed() const
	{
		return m_IsCompressed;
	}

	bool Image::IsCubeMap() const
	{
		return m_IsCubeMap;
	}

	void Image::SetName(const string & _Name)
	{
		m_ImgName = _Name;
	}

	Image::string Image::GetName() const
	{
		return m_ImgName;
	}

	uint32 Image::GetWidth()const
	{
		return m_ImgWidth;
	}

	uint32 Image::GetHeight() const
	{
		return m_ImgHeight;
	}

	uint32 Image::GetDepth() const
	{
		return m_ImgDepth;
	}

	uint32 Image::GetMipLevs() const
	{
		return m_MipLev;
	}

	uint32 Image::GetLayers() const
	{
		return m_ImgLayers;
	}

	uint32 Image::GetInternalFmt() const
	{
		return m_InternalFmt;
	}

	uint32 Image::GetFillFmt() const
	{
		return m_FillFmt;
	}

	uint32 Image::GetDataType() const
	{
		return m_DataType;
	}

	kByte* Image::GetData() const
	{
		return &m_ImgData[0][0];
	}
}