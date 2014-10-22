#include "kGLRenderTexture.h"
#include "kGLTexture.h"
#include "kGLFrameBuffer.h"

#define SAFERELEASE(x)  if((x)) { delete (x); (x) = nullptr; }

kGLRenderTexture::kGLRenderTexture()
{
  m_ColorTexture  = nullptr;
  m_DepthTexture  = nullptr;
  m_FrameBuffer   = nullptr;
}

kGLRenderTexture::~kGLRenderTexture()
{

}

void kGLRenderTexture::Clear()
{
  m_RTType = RT_2D;
  m_RTFlags = 0;
  m_Width = 1;
  m_Height = 1;
  m_Depth = 1;
  m_Layers = 1;
  SAFERELEASE(m_ColorTexture);
  SAFERELEASE(m_DepthTexture);
  SAFERELEASE(m_FrameBuffer);
}

int kGLRenderTexture::Create2DRT(int width, int height, int flags)
{
  assert(width > 0 && height > 0 && "kGLRenderTexture::Create2DRT(): bad texture render size");
  Clear();
  m_RTType = RT_2D;
  m_RTFlags = flags;
  m_Width = width;
  m_Height = height;
  return create();
}

int kGLRenderTexture::Create3DRT(int width, int height, int depth, int flags)
{
  assert(width > 0 && height > 0 && depth > 0 && "kGLRenderTexture::Create3DRT(): bad texture render size");
  Clear();
  m_RTType = RT_3D;
  m_RTFlags = flags;
  m_Width = width;
  m_Height = height;
  m_Depth = depth;
  return create();
}

int kGLRenderTexture::CreateCubeRT(int width, int height, int flags)
{
  assert(width > 0 && height > 0 && "kGLRenderTexture::CreateCubeRT(): bad texture render size");
  Clear();
  m_RTType = RT_Cube;
  m_RTFlags = flags;
  m_Width = width;
  m_Height = height;
  m_Layers = 6;
  return create();
}

int kGLRenderTexture::Create2DArrayRT(int width, int height, int num_layers, int flags)
{
  assert(width > 0 && height > 0 && "kGLRenderTexture::CreateCubeRT(): bad texture render size");
  Clear();
  m_RTType = RT_2DArray;
  m_RTFlags = flags;
  m_Width = width;
  m_Height = height;
  m_Layers = num_layers;
  return create();
}

void kGLRenderTexture::Release()
{
  if(m_ColorTexture) {
    m_ColorTexture->Release();
    delete m_ColorTexture;
    m_ColorTexture = nullptr;
  }

  if(m_DepthTexture) {
    m_DepthTexture->Release();
    delete m_DepthTexture;
    m_DepthTexture = nullptr;
  }

  if(m_FrameBuffer) {
    m_FrameBuffer->Release();
    delete m_FrameBuffer;
    m_FrameBuffer = nullptr;
  }

  create();
}

bool kGLRenderTexture::IsCompleted() const
{
  return m_FrameBuffer->IsCompleted();
}

void kGLRenderTexture::Bind()
{
  m_FrameBuffer->Enable();
}

void kGLRenderTexture::UnBind()
{

}

int kGLRenderTexture::create()
{
  int framebuffer_flags = 0;

  if(m_RTFlags & Multisample_2) framebuffer_flags |= Multisample_2;
  else if(m_RTFlags & Multisample_4) framebuffer_flags |= Multisample_4;
  else if(m_RTFlags & Multisample_8) framebuffer_flags |= Multisample_8;

  // color texture
  if(m_RTFlags & RT_Tex_Color) {

      int texture_format = -1;
      if(m_RTFlags & RT_Color_R5G6B5) texture_format = Fmt_R5G6B5;
      else if(m_RTFlags & RT_Color_RGBA8) texture_format = Fmt_RGBA8;
      else if(m_RTFlags & RT_Color_RGB10A2) texture_format = Fmt_RGB10A2;
      else if(m_RTFlags & RT_Color_RGBA16) texture_format = Fmt_RGBA16;
      else if(m_RTFlags & RT_Color_RGBA16F) texture_format = Fmt_RGBA16F;
      else if(m_RTFlags & RT_Color_RGBA32F) texture_format = Fmt_RGBA32F;
      else if(m_RTFlags & RT_Color_RG11B10F) texture_format = Fmt_RG11B10F;

      if(texture_format != -1) {
          int texture_flags = 0;
          if(!(m_RTFlags & RT_Wrap_Repeat))
            texture_flags |= Wrap_Clamp;
          // set filter
          if(!(m_RTFlags & RT_Filter_Linear))
            texture_flags |= Filter_Linear;
          else
            texture_flags |= Filter_Linear;

          // set MultiSample
          if( m_RTFlags & Multisample_2 )
            texture_flags |= Multisample_2;
          else if( m_RTFlags & Multisample_4 )
            texture_flags |= Multisample_4;
          else if( m_RTFlags & Multisample_8 )
            texture_flags |= Multisample_8;

          // now create texture
          m_ColorTexture = createTexture(texture_format, texture_flags);
      }
  }

  if(GetMultiSamples() || (m_RTFlags & RT_Tex_Color) == 0) {
      if(m_RTFlags & RT_Color_R5G6B5) framebuffer_flags |= RT_Color_R5G6B5;
      else if(m_RTFlags & RT_Color_RGBA8) framebuffer_flags |= RT_Color_RGBA8;
      else if(m_RTFlags & RT_Color_RGB10A2) framebuffer_flags |= RT_Color_RGB10A2;
      else if(m_RTFlags & RT_Color_RGBA16) framebuffer_flags |= RT_Color_RGBA16;
      else if(m_RTFlags & RT_Color_RGBA16F) framebuffer_flags |= RT_Color_RGBA16F;
      else if(m_RTFlags & RT_Color_RGBA32F) framebuffer_flags |= RT_Color_RGBA32F;
      else if(m_RTFlags & RT_Color_RG11B10F) framebuffer_flags |= RT_Color_RG11B10F;

  }

  if(m_RTFlags & RT_Tex_Depth) {
      int texture_format = -1;
      if(m_RTFlags & RT_Depth_16) texture_format = Fmt_D16;
      else if((m_RTFlags & RT_Depth_16) && (m_RTFlags & RT_Stencil_8)) texture_format = Fmt_D24S8;
      else if(m_RTFlags & RT_Depth_24) texture_format = Fmt_D24;
      else if((m_RTFlags & RT_Depth_32F) && (m_RTFlags & RT_Stencil_8)) texture_format = Fmt_D32FS8;
      else if(m_RTFlags & RT_Depth_32F) texture_format = Fmt_D32F;

      if(texture_format != -1) {
          int texture_flags = 0;
          if((m_RTFlags & RT_Wrap_Repeat) == 0) texture_flags |= Wrap_Clamp;
          if((m_RTFlags & Filter_Linear) == 0) texture_flags |= Filter_Nearest;
          else texture_flags |= Filter_Linear;

          if(m_RTFlags & Multisample_2) texture_flags |= Multisample_2;
          else if(m_RTFlags & Multisample_4) texture_flags |= Multisample_4;
          else if(m_RTFlags & Multisample_8) texture_flags |= Multisample_8;

          m_DepthTexture = createTexture(texture_format, texture_flags);
      }
  }

  if(GetMultiSamples() || ( m_RTFlags & RT_Tex_Depth ) == 0) {
      if(m_RTFlags & RT_Depth_16) {
          framebuffer_flags |= RT_Depth_16;
      } else if((m_RTFlags & RT_Depth_24) && (m_RTFlags & RT_Stencil_8)) {
          m_DepthTexture = new kGLTexture;
          int texture_flags = Wrap_Clamp | Filter_Nearest;
          if(m_RTFlags & Multisample_2) texture_flags |= Multisample_2;
          else if(m_RTFlags & Multisample_4) texture_flags |= Multisample_4;
          else if(m_RTFlags & Multisample_8) texture_flags |= Multisample_8;
          m_DepthTexture->Create2D(m_Width, m_Height, Fmt_D24S8, texture_flags);
      } else if(m_RTFlags & RT_Depth_24) {
          framebuffer_flags |= RT_Depth_24;
      } else if((m_RTFlags & RT_Depth_32F) && (m_RTFlags & RT_Stencil_8)) {
          m_DepthTexture = new kGLTexture();
          int texture_flags = Wrap_Clamp | Filter_Nearest;
          if(m_RTFlags & Multisample_2) texture_flags |= Multisample_2;
          else if(m_RTFlags & Multisample_4) texture_flags |= Multisample_4;
          else if(m_RTFlags & Multisample_8) texture_flags |= Multisample_8;
          m_DepthTexture->Create2D(m_Width, m_Height, Fmt_D32FS8, texture_flags);
      } else if(m_RTFlags & RT_Depth_32F) {
          framebuffer_flags |= RT_Depth_32F;
      }
  }
  m_FrameBuffer = new kGLFrameBuffer(m_Width, m_Height, framebuffer_flags);
  m_FrameBuffer->SetColorTexture(m_ColorTexture, 0);
  m_FrameBuffer->SetDepthTexture(m_DepthTexture);

  return 1;
}

kGLTexture *kGLRenderTexture::createTexture(int format, int flags)
{
  kGLTexture *texture = new kGLTexture;
  switch(m_RTType) {
  case RT_2D:
		texture->Create2D(m_Width, m_Height, format, flags);
    break;
  case RT_3D:
		texture->Create3D(m_Width, m_Height, m_Depth, format, flags);
    break;
  case RT_Cube:
		texture->CreateCube(m_Width, m_Height, format, flags);
    break;
  case RT_2DArray:
		texture->Create2DArray(m_Width, m_Height, m_Layers, format, flags);
    break;
  default:
    assert(0 && "kGLRenderTexture::create(): unknown texture render type");
    break;
  }
  return texture;
}

void kGLRenderTexture::Enable()
{
	assert(m_FrameBuffer != nullptr && "m_FrameBuffer not aloocated !");
	m_FrameBuffer->Enable();
}

void kGLRenderTexture::Disable()
{
	assert(m_FrameBuffer != nullptr && "m_FrrameBuffer not allocated !");
	if (m_FrameBuffer->IsEnabled())
		m_FrameBuffer->Disable();
}

void kGLRenderTexture::Flush()
{
	assert(m_FrameBuffer != nullptr && "m_FrrameBuffer not allocated !");
	m_FrameBuffer->Flush();
}
