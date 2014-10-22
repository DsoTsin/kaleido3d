#ifndef __RenderEnums_h__
#define __RenderEnums_h__
#pragma once


enum {
  Tex_1D,
  Tex_2D,
  Tex_3D,
  Tex_Cube,
  Tex_2DArray
};

// for tex format
enum {
  Fmt_R8 = 0,
  Fmt_RG8,
  Fmt_RGB8,
  Fmt_RGBA8,

  Fmt_R16,
  Fmt_RG16,
  Fmt_RGB16,
  Fmt_RGBA16,

  Fmt_R16F,
  Fmt_RG16F,
  Fmt_RGB16F,
  Fmt_RGBA16F,

// hight precision
  Fmt_R32F,
  Fmt_RG32F,
  Fmt_RGB32F,
  Fmt_RGBA32F,

// specified
  Fmt_R5G6B5,
//    Fmt_RGBA4,
//    Fmt_RGB5A1,
  Fmt_RGB10A2,
  Fmt_RG11B10F,

// Compressed
  Fmt_DXT1,
  Fmt_DXT3,
  Fmt_DXT5,

// depth
  Fmt_D16,
  Fmt_D24,
  Fmt_D24S8,
  Fmt_D32F,
  Fmt_D32FS8,

  UNKNOWN
};

//for tex flag
enum {
  Wrap_ClampX = 1 << 4,
  Wrap_ClampY = 1 << 5,
  Wrap_ClampZ = 1 << 6,
  Wrap_Clamp = (Wrap_ClampX | Wrap_ClampY | Wrap_ClampZ),

  Wrap_BorderX = 1 << 7,
  Wrap_BorderY = 1 << 8,
  Wrap_BorderZ = 1 << 9,
  Wrap_BorderW = 1 << 10,
  Wrap_Border = (Wrap_BorderX | Wrap_BorderY | Wrap_BorderZ),
  Wrap_Mask = (Wrap_Clamp | Wrap_Border | Wrap_BorderW),

  Filter_Nearest = 1 << 11,
  Filter_Linear = 1 << 12,
  Filter_Bilinear = 1 << 13,
  Filter_Trilinear = 1 << 14,
  Filter_Mask = (Filter_Nearest | Filter_Linear | Filter_Bilinear | Filter_Trilinear),

  Anisotropy_1 = 1 << 15,
  Anisotropy_2 = 1 << 16,
  Anisotropy_4 = 1 << 17,
  Anisotropy_8 = 1 << 18,
  Anisotropy_16 = 1 << 19,
  Anisotropy_Mask = (Anisotropy_1 | Anisotropy_2 | Anisotropy_4 | Anisotropy_8 | Anisotropy_16),

  Multisample_2 = 1 << 20,
  Multisample_4 = 1 << 21,
  Multisample_8 = 1 << 22,
  Multisample_Mask = (Multisample_2 | Multisample_4 | Multisample_8),

  Shadow_Compare = 1 << 23,
  Shadow_Mask = (Shadow_Compare),

  Default_Flag = (Filter_Linear),

  Num_Flags = 24,
};


enum {
  RT_2D = 0, RT_3D, RT_Cube, RT_2DArray
};

// for renderTarget
enum {
  RT_Color_R5G6B5 = 1 << 0, RT_Color_RGBA8 = 1 << 1, RT_Color_RGB10A2 = 1 << 2, RT_Color_RGBA16 = 1 << 3, RT_Color_RGBA16F = 1 << 4, RT_Color_RGBA32F = 1 << 5, RT_Color_RG11B10F = 1 << 6,
  RT_Depth_16 = 1 << 7, RT_Depth_24 = 1 << 8, RT_Depth_32F = 1 << 9, RT_Stencil_8 = 1 << 10,
  RT_Tex_Color = 1 << 14, RT_Tex_Depth = 1 << 15,
  RT_Wrap_Repeat = 1 << 16, RT_Filter_Linear = 1 << 17,
  // if multisample then compose with Multisample before 20 bits
};

#endif
