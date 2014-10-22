#pragma once
#include <Core/k3dImage.h>

#include "RenderEnums.h"

class k3dTexture {
public:

  k3dTexture() { m_VMUsed = 0; }
  virtual ~k3dTexture() {}

  virtual void Bind(uint32 textureUnit) = 0;

  virtual void Release() = 0;

  virtual int Create(
      const k3dImage & image,
      uint32 flag = Default_Flag) = 0;

  virtual int Create1D(
      int width,
      uint32 format,
      uint32 flag) = 0;
  virtual int Create2D(
      int width, int height,
      uint32 format,
      uint32 flag = Default_Flag) = 0;
  virtual int Create3D(
      int width, int height, int depth,
      uint32 format,
      uint32 flag = Default_Flag) = 0;
  virtual int CreateCube(
      int width, int height,
      uint32 format,
      uint32 flag = Default_Flag) = 0;
  virtual int Create2DArray(
      int width, int height, int num_layers,
      uint32 format,
      uint32 flag = Default_Flag) = 0;

  virtual void SetFlag(uint32 flag) = 0;
  virtual void SetPixelAlignment(uint32 align) = 0;

  uint32 GetType() const { return m_Type; }
  uint32 GetFormat() const { return m_Format; }
//  virtual bool isCompressed() const = 0;
  bool IsDepthFormat() const {
    return (m_Format >= Fmt_D16 && m_Format <= Fmt_D32FS8 );
  }

  bool IsColorFormat() const {
    return (m_Format>=Fmt_R8 && m_Format<=Fmt_DXT5);
  }

  uint32 GetWidth() const {
    return m_Width;
  }

  uint32 GetHeight() const {
    return m_Height;
  }

  uint32 GetDepth() const {
    return m_Depth;
  }

  uint32 GetLayers() const {
    return m_Layers;
  }

  uint32 GetFlags() const {
    return m_Flags;
  }

protected:

  uint32 m_Type;
  uint32 m_Format;
  uint32 m_Flags;

  uint32 m_Width;
  uint32 m_Height;
  uint32 m_Depth;

  uint32 m_Mips;
  uint32 m_Layers;

  uint32 m_VMUsed;
};

typedef std::shared_ptr<k3dTexture> kTexPtr;
