#include "kGLTexture.h"
#include <GL/glew.h>

//-------------------------------------------------------------

kGLTexture::kGLTexture()
  : m_TexId(-1)
{
}

kGLTexture::~kGLTexture()
{
}

void kGLTexture::Bind(uint32 textureUnit)
{
  glActiveTexture(GL_TEXTURE0+textureUnit);
  Bind();
}

void kGLTexture::Bind()
{
  glBindTexture(m_TexType, m_TexId);
}

void kGLTexture::UnBind()
{
  glBindTexture(m_TexType, 0);
}

void kGLTexture::Release()
{
  m_Type = Tex_2D;
  m_Format = Fmt_RGBA8;
  m_Flags = 0;

  m_Width = 1;
  m_Height = 1;
  m_Depth = 1;
  m_Mips = 1;
  m_Layers = 1;
  m_VMUsed = 0;

  m_TexType = 0;
  m_TexInternalFormat = 0;

  if(::glIsTexture(m_TexId)) {
    ::glDeleteTextures(1, &m_TexId);
  }
  m_TexId = 0;

}

int kGLTexture::Create(const k3dImage &image, uint32 flag)
{
  glGenTextures(1, &m_TexId);
  uint32 internalFormat = image.GetInternalFmt();
  m_TexInternalFormat = internalFormat;
//  m_Format;
  m_Flags = flag;
  m_Width = image.GetWidth();
  m_Height = image.GetHeight();
  m_Depth = image.GetDepth();
  m_Mips = image.GetMipLevs();
  m_Layers = image.GetLayers();

  if (image.IsCubeMap()) {
    m_Type = Tex_Cube;
    m_TexType = GL_TEXTURE_CUBE_MAP;
    int32 error = glGetError();
    assert(error==GL_NO_ERROR && "kGLTexture isCubeMap Error.");
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_TexId);
    error = glGetError();
    assert(error==GL_NO_ERROR && "kGLTexture Bind CubeMap Error.");
    for (int32 f = GL_TEXTURE_CUBE_MAP_POSITIVE_X; f <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; f++) {
      uint32 w = image.GetWidth();
      uint32 h = image.GetHeight();
      for (uint32 l = 0; l < image.GetMipLevs(); l++) {
        if (image.IsCompressed()) {
          glCompressedTexImage2D( f, l, internalFormat, w, h,
                                  0, image.GetImageSize(l), image.GetLevel(l, f-GL_TEXTURE_CUBE_MAP_POSITIVE_X));
        } else {
          glTexImage2D( f, l, internalFormat, w, h, 0,
                        image.GetFillFmt(), image.GetDataType(), image.GetLevel(l, f-GL_TEXTURE_CUBE_MAP_POSITIVE_X));
        }
        error = glGetError();
        assert(error==GL_NO_ERROR && "kGLTexture Transfer CubeMap Error.");
        w >>= 1;
        h >>= 1;
        w = w ? w : 1;
        h = h ? h : 1;
      }
    }
  }
  else {
    int32 error = glGetError();
    m_Type = Tex_2D;
    m_TexType = GL_TEXTURE_2D;
    glBindTexture(GL_TEXTURE_2D, m_TexId);

    uint32 w = image.GetWidth();
    uint32 h = image.GetHeight();
    for (uint32 l = 0; l < image.GetMipLevs(); l++) {
      if (image.IsCompressed()) {
        glCompressedTexImage2D( GL_TEXTURE_2D, l, internalFormat, w, h,
                                0, image.GetImageSize(l), image.GetLevel(l, 0));
      } else {
        glTexImage2D( GL_TEXTURE_2D, l, internalFormat, w, h, 0,
                      image.GetFillFmt(), image.GetDataType(), image.GetLevel(l, 0));
      }
      error = glGetError();
      w >>= 1;
      h >>= 1;
      w = w ? w : 1;
      h = h ? h : 1;
      assert(error==GL_NO_ERROR && "kGLTexture transfer 2D texture Error.");
    }
  }

  SetFlag(flag);
  int32 error = glGetError();
  assert(error==GL_NO_ERROR && "kGLTexture SetFlag Error.");

  return 1;
}

int kGLTexture::Create1D(int width, uint32 format, uint32 flag)
{
  assert(width > 0 && "kGLTexture::Create1D(): bad texture size");

  Release();

  m_Type = Tex_2D;
  m_Format = format;

  m_Width = width;
  m_TexType = GL_TEXTURE_1D;

  glGenTextures(1, &m_TexId);
  SetFlag(flag);

  if(UpdateFormat() == 0) return 0;

  glBindTexture(m_Type, m_TexId);
  glTexStorage1D( m_TexType, 0, m_TexInternalFormat, width );

  return 1;
}

int kGLTexture::Create2D(int width, int height, uint32 format, uint32 flag)
{
  assert(width > 0 && height > 0 && "kGLTexture::Create2D(): bad texture size");

  Release();

  m_Type = Tex_2D;
  m_Format = format;

  m_Width = width;
  m_Height = height;

  int samples = 0;
  if(flag & Multisample_2) samples = 2;
  else if(flag & Multisample_4) samples = 4;
  else if(flag & Multisample_8) samples = 8;

  m_TexType = ( samples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D );

  glGenTextures(1, &m_TexId);
  SetFlag(flag);

  if(UpdateFormat() == 0) return 0;

  glBindTexture(m_Type, m_TexId);
  samples ? glTexImage2DMultisample(m_TexType, samples, m_TexInternalFormat, width, height, GL_TRUE)
          : glTexStorage2D(m_TexType, 0, m_TexInternalFormat, width, height);

  if(flag & Shadow_Compare) {
      glTexParameteri(m_TexType, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      glTexParameteri(m_TexType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  }

  return 1;
}

int kGLTexture::Create3D(int width, int height, int depth, uint32 format, uint32 flag)
{
  assert(width > 0 && height > 0 && depth > 0 && "kGLTexture::Create3D(): bad texture size");

  Release();

  m_Type    = Tex_3D;
  m_Format  = format;
  m_Width   = width;
  m_Height  = height;
  m_Depth   = depth;
  m_TexType = GL_TEXTURE_3D;

  glGenTextures(1,&m_TexId);
  SetFlag(flag);

  if(UpdateFormat() == 0) return 0;
  glBindTexture(m_TexType, m_TexId);
  glTexStorage3D(m_TexType, 0, m_TexInternalFormat, width, height, depth);

  return 1;
}

int kGLTexture::CreateCube(int width, int height, uint32 format, uint32 flag)
{
  K3D_UNUSED(width);
  K3D_UNUSED(height);
  K3D_UNUSED(format);
  K3D_UNUSED(flag);
  return 1;
}

int kGLTexture::Create2DArray(int width, int height, int num_layers, uint32 format, uint32 flag)
{
  return 1;
}

int kGLTexture::UpdateFormat()
{
  static const uint32 glInternalFormats[] = {
    GL_R8,
    GL_RG8,
    GL_RGB8, // most common image format like bmps
    GL_RGBA8, // with alpha channel like pngs

    GL_R16,
    GL_RG16,
    GL_RGB16,
    GL_RGBA16,

    GL_R16F,
    GL_RG16F, // rotation tex
    GL_RGB16F,
    GL_RGBA16F,

// high precision
    GL_R32F,
    GL_RG32F,
    GL_RGB32F,
    GL_RGBA32F,

// specified
    GL_RGB5,
    GL_RGB10_A2, // gbuffer
    GL_R11F_G11F_B10F,

// compressed
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,

// depth
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,
    GL_DEPTH24_STENCIL8,
    GL_DEPTH_COMPONENT32F,
    GL_DEPTH32F_STENCIL8

  };

  if( m_Format >= UNKNOWN )
    return 0;

  m_TexInternalFormat = glInternalFormats[m_Format];
  return 1;
}

void kGLTexture::SetFlag(uint32 flag)
{
  m_Flags = flag;
  ::glBindTexture(m_TexType, m_TexId);

  if((flag & Multisample_Mask) == 0) {

      if( flag & Wrap_ClampX )
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      else if( flag & Wrap_BorderX )
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      else
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_S, GL_REPEAT);

      if(flag & Wrap_ClampY)
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      else if(flag & Wrap_BorderY)
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      else
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_T, GL_REPEAT);

      if(flag & Wrap_ClampZ)
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      else if(flag & Wrap_BorderZ)
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
      else
        ::glTexParameteri(m_TexType, GL_TEXTURE_WRAP_R, GL_REPEAT);

      if(flag & Wrap_BorderW) {
          const GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
          ::glTexParameterfv(m_TexType, GL_TEXTURE_BORDER_COLOR, color);
      } else {
          const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
          ::glTexParameterfv(m_TexType, GL_TEXTURE_BORDER_COLOR, color);
      }

      if(flag & Filter_Nearest) {
          glTexParameteri(m_TexType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(m_TexType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      } else if(flag & Filter_Linear) {
          glTexParameteri(m_TexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(m_TexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      } else if(flag & Filter_Bilinear) {
          glTexParameteri(m_TexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
          glTexParameteri(m_TexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      } else if(flag & Filter_Trilinear) {
          glTexParameteri(m_TexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glTexParameteri(m_TexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }

      if((flag & Filter_Nearest) == 0) {
          if(flag & Anisotropy_2)
            ::glTexParameterf(m_TexType, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
          else if(flag & Anisotropy_4)
            ::glTexParameterf(m_TexType, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
          else if(flag & Anisotropy_8)
            ::glTexParameterf(m_TexType, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
          else if(flag & Anisotropy_16)
            ::glTexParameterf(m_TexType, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
          else
            ::glTexParameterf(m_TexType,GL_TEXTURE_MAX_ANISOTROPY_EXT,1.0f);
      } else {
          ::glTexParameterf(m_TexType,GL_TEXTURE_MAX_ANISOTROPY_EXT,1.0f);
      }
  }

  ::glBindTexture(m_TexType, 0);
}

void kGLTexture::SetPixelAlignment(uint32 align)
{
  ::glPixelStorei(GL_UNPACK_ALIGNMENT, align);
}

uint32 kGLTexture::GetTexType() const
{
  return m_TexType;
}

uint32 kGLTexture::GetTexId() const
{
  return m_TexId;
}

kGLImmuTexture::kGLImmuTexture()
{

}

kGLImmuTexture::~kGLImmuTexture()
{

}

int kGLImmuTexture::Create1D(int width, uint32 format, uint32 flag)
{
  return 1;
}

int kGLImmuTexture::Create2D(int width, int height, uint32 format, uint32 flag)
{
  assert(width > 0 && height > 0 && "kGLImmuTexture::Create2D(): bad texture size");
  Release();
  m_Type = Tex_2D;
  m_Format = format;

  m_Width = width;
  m_Height = height;

  int samples = 0;
  if(flag & Multisample_2) samples = 2;
  else if(flag & Multisample_4) samples = 4;
  else if(flag & Multisample_8) samples = 8;

  m_TexType = ( samples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D );

  glGenTextures(1, &m_TexId);
  SetFlag(flag);

  if(UpdateFormat() == 0) return 0;

  glBindTexture(m_Type, m_TexId);
  samples ? glTexImage2DMultisample(m_TexType, samples, m_TexInternalFormat, width, height, GL_TRUE)
          : glTexStorage2D(m_TexType, 0, m_TexInternalFormat, width, height);

  if(flag & Shadow_Compare) {
      glTexParameteri(m_TexType, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      glTexParameteri(m_TexType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  }

  return 1;
}

int kGLImmuTexture::Create3D(int width, int height, int depth, uint32 format, uint32 flag)
{
  return 1;
}

int kGLImmuTexture::CreateCube(int width, int height, uint32 format, uint32 flag)
{
  return 1;
}

int kGLImmuTexture::Create2DArray(int width, int height, int num_layers, uint32 format, uint32 flag)
{
  return 1;
}
