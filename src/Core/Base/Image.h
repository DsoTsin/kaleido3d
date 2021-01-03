#pragma once
#ifndef __k3d_Base_Image_h__
#define __k3d_Base_Image_h__

namespace k3d {
enum class ImageFileFormat {
  PNG,
  JPEG,
  TGA,
  TIFF,
  BMP,
  _Unknown,
};

class K3D_CORE_API ImageFactory {
public:
  static ImageFactory &g();

private:
  ImageFactory();
  ~ImageFactory();
};

enum class PixelFormat {
  RGBA8,
};

class K3D_CORE_API Image {
public:
private:
};

} // namespace k3d

#endif
