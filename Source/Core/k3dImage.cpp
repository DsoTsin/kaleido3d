#include "k3dImage.h"

//---------------------------------------------------------------
// DDS image loader implement ??

k3dImage::k3dImage()
  : m_ImgWidth(0)
  , m_ImgHeight(0)
  , m_ImgLayers(0)
  , m_MipLev(0)
{
}

k3dImage::~k3dImage()
{
  if(!m_ImgData.empty()) m_ImgData.clear();
}

uint32 k3dImage::GetImageSize(uint32 level) const
{
  bool compressed = IsCompressed();
  int32_t w = m_ImgWidth >> level;
  int32_t h = m_ImgHeight >> level;
  int32_t d = m_ImgDepth >> level;
  w = (w) ? w : 1;
  h = (h) ? h : 1;
  d = (d) ? d : 1;
  int32_t bw = (compressed) ? ( w + 3 ) / 4 : w;
  int32_t bh = (compressed) ? ( h + 3 ) / 4 : h;
  int32_t elementSize = m_ElementSize;

  return bw*bh*d*elementSize;
}

const void *k3dImage::GetLevel(uint32 level, uint32 face) const
{
  assert( level < (uint32)m_MipLev );
//  assert( face >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
//  assert( face == GL_TEXTURE_CUBE_MAP_POSITIVE_X || _cubeMap);
//  face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  assert( (face*m_MipLev + level) < (int32)m_ImgData.size());
  if (level >= (uint32)m_MipLev || (int32)face >= m_ImgLayers)
    return nullptr;

  return m_ImgData[ face*m_MipLev + level];
}

bool k3dImage::Load(uint8 *dataPtr, uint32 length)
{
  K3D_UNUSED(dataPtr);
  K3D_UNUSED(length);
  return false;
}

bool k3dImage::IsCompressed() const
{
  return m_IsCompressed;
}

bool k3dImage::IsCubeMap() const
{
  return m_IsCubeMap;
}

void k3dImage::SetName(const k3dString & _Name)
{
  m_ImgName = _Name;
}

 k3dString k3dImage::GetName() const
{
  return m_ImgName;
}

 uint32 k3dImage::GetWidth()const
{
  return m_ImgWidth;
}

 uint32 k3dImage::GetHeight() const
{
  return m_ImgHeight;
}

 uint32 k3dImage::GetDepth() const
{
  return m_ImgDepth;
}

 uint32 k3dImage::GetMipLevs() const
{
  return m_MipLev;
}

 uint32 k3dImage::GetLayers() const
{
  return m_ImgLayers;
}

 uint32 k3dImage::GetInternalFmt() const
{
  return m_InternalFmt;
}

 uint32 k3dImage::GetFillFmt() const
{
  return m_FillFmt;
}

 uint32 k3dImage::GetDataType() const
{
  return m_DataType;
}

kByte* k3dImage::GetData() const
{
  return &m_ImgData[0][0];
}
