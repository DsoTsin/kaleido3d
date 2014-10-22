#include "kGLFrameBuffer.h"
#include <GL/glew.h>
#include <Core/k3dLog.h>
#include <Core/k3dDbg.h>
//#include <Renderer/OpenGL/kGLTexture.h>

kGLFrameBuffer::kGLFrameBuffer(int width, int height, uint32 flags)
  : m_FrameBufferId(-1)
{
  m_FrameBufferPreviousId = -1;
	m_BlitDrawBuffer = -1;

  m_DepthTexture = nullptr;

  m_Enabled = 0;
  m_MemUsage = 0;

	for (int i = 0; i < NUM_RT_TEX; i++)
		m_ColorTextures[i] = nullptr;

  // get previous frame buffer id
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_FrameBufferPreviousId);

	if (m_Samples != 0) glGenFramebuffers(1, &m_BlitDrawBuffer);

	glGenFramebuffers(1, &m_FrameBufferId);
  glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);

  // color render buffer
  GLuint internal_format = 0;
  if(flags & RT_Color_R5G6B5) internal_format = GL_RGB5;
  else if(flags & RT_Color_RGBA8) internal_format = GL_RGBA8;
  else if(flags & RT_Color_RGB10A2) internal_format = GL_RGB10_A2;
  else if(flags & RT_Color_RGBA16) internal_format = GL_RGBA16;
  else if(flags & RT_Color_RGBA16F) internal_format = GL_RGBA16F;
  else if(flags & RT_Color_RGBA32F) internal_format = GL_RGBA32F;
  else if(flags & RT_Color_RG11B10F) internal_format = GL_R11F_G11F_B10F;

  if(internal_format) {
      glGenRenderbuffers(1, &m_ColorBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_ColorBufferId);
      if(m_Samples == 0) glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
      else glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_Samples, internal_format, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBufferId);
      if(internal_format == GL_RGB5) m_MemUsage += width * height * 2;
      else if(internal_format == GL_RGBA8) m_MemUsage += width * height * 4;
      else if(internal_format == GL_RGB10_A2) m_MemUsage += width * height * 4;
      else if(internal_format == GL_RGBA16) m_MemUsage += width * height * 8;
      else if(internal_format == GL_RGBA16F) m_MemUsage += width * height * 8;
      else if(internal_format == GL_RGBA32F) m_MemUsage += width * height * 16;
      else if(internal_format == GL_R11F_G11F_B10F) m_MemUsage += width * height * 4;
      else assert(0 && "GLFrameBuffer::GLFrameBuffer(): unknown color internal format");
      glBindRenderbuffer(GL_RENDERBUFFER,0);
  }

  // depth render buffer
  internal_format = 0;
  if(flags & RT_Depth_16) internal_format = GL_DEPTH_COMPONENT16;
  else if(flags & RT_Depth_24) internal_format = GL_DEPTH_COMPONENT24;
  else if(flags & RT_Depth_32F) internal_format = GL_DEPTH_COMPONENT32F;

  if(internal_format) {
      glGenRenderbuffers(1, &m_DepthBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBufferId);

      if(m_Samples == 0) glRenderbufferStorage(GL_RENDERBUFFER,internal_format,width,height);
      else glRenderbufferStorageMultisample(GL_RENDERBUFFER,m_Samples,internal_format,width,height);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferId);

      if(internal_format == GL_DEPTH_COMPONENT16) m_MemUsage += width * height * 2;
      else if(internal_format == GL_DEPTH_COMPONENT24) m_MemUsage += width * height * 4;
      else if(internal_format == GL_DEPTH_COMPONENT32F) m_MemUsage += width * height * 4;
      else assert(0 && "kGLFrameBuffer::kGLFrameBuffer(): unknown depth internal format");
      glBindRenderbuffer(GL_RENDERBUFFER,0);
  }

  // stencil render buffer
  internal_format = 0;
  if(flags & RT_Stencil_8) internal_format = GL_STENCIL_INDEX8;
  if(internal_format) {
      glGenRenderbuffers(1, &m_StencilBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_StencilBufferId);
      m_Samples == 0  ? glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height)
                      : glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_Samples, internal_format, width, height);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBufferId);
      if(internal_format == GL_STENCIL_INDEX8) m_MemUsage += width * height * 1;
      else assert(0 && "kGLFrameBuffer::kGLFrameBuffer(): unknown stencil internal format");
      glBindRenderbuffer(GL_RENDERBUFFER,0);
  }

// restore framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferPreviousId);
}

kGLFrameBuffer::~kGLFrameBuffer()
{
	Release();
}

void kGLFrameBuffer::Release()
{
  if(glIsFramebuffer(m_FrameBufferId))
		glDeleteFramebuffers(1, &m_FrameBufferId);

	if (glIsFramebuffer(m_BlitDrawBuffer))
		glDeleteFramebuffers(1, &m_BlitDrawBuffer);

  if(glIsRenderbuffer(m_ColorBufferId))
    glDeleteRenderbuffers(1, &m_ColorBufferId);

  if(glIsRenderbuffer(m_DepthBufferId))
    glDeleteRenderbuffers(1, &m_DepthBufferId);

  if(glIsRenderbuffer(m_StencilBufferId))
    glDeleteRenderbuffers(1, &m_StencilBufferId);
}

void kGLFrameBuffer::SetDepthTexture(kGLTexture *texture)
{
  if( texture && !texture->IsDepthFormat() ) {
    k3dLog::Error("kGLFrameBuffer::setDepthTexture(): unsupported format.");
    return;
  }
  m_DepthTexture = texture;
  if(m_Enabled && m_Samples == 0) {
    if(texture) {
      if( texture->GetType() == Tex_2D ) {
        if(texture->GetFormat() == Fmt_D16 || texture->GetFormat() == Fmt_D24 || texture->GetFormat() == Fmt_D32F) {
          glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexType(),texture->GetTexId(),0);
          glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,GL_RENDERBUFFER,m_StencilBufferId);
        } else if(texture->GetFormat() == Fmt_D24S8 || texture->GetFormat() == Fmt_D32FS8) {
          glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, texture->GetTexType(), texture->GetTexId(), 0);
          glFramebufferTexture2D(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT, texture->GetTexType(), texture->GetTexId(), 0);
        } else {
          k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unsupported format. ");
        }
      }
      else if( texture->GetType() == Tex_Cube ) {
        if(texture->GetFormat() == Fmt_D16 || texture->GetFormat() == Fmt_D24 || texture->GetFormat() == Fmt_D32F) {
          if(m_Layers == -1) {
            glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0);
          } else {
            assert(m_Layers >= 0 && m_Layers < 6 && "kGLFrameBuffer::SetDepthTexture(): bad m_Layers number");
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_Layers);
          }
          glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBufferId);
        } else if(texture->GetFormat() == Fmt_D24S8 || texture->GetFormat() == Fmt_D32FS8) {
          if(m_Layers == -1) {
            glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0);
            glFramebufferTexture(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,texture->GetTexId(),0);
          } else {
            assert(m_Layers >= 0 && m_Layers < 6 && "kGLFrameBuffer::SetDepthTexture(): bad m_Layers number");
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_Layers);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,texture->GetTexId(),0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_Layers);
          }
        } else {
          k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unsupported format .");
        }
      }
      else if( texture->GetType() == Tex_2DArray ) {
        if(texture->GetFormat() == Fmt_D16 || texture->GetFormat() == Fmt_D24 || texture->GetFormat() == Fmt_D32F) {
          if(m_Layers == -1) {
            glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0);
          } else {
            assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetDepthTexture(): bad m_Layers number");
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->GetTexId(), 0, m_Layers);
          }
          glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,GL_RENDERBUFFER,m_StencilBufferId);
        } else if(texture->GetFormat() == Fmt_D24S8 || texture->GetFormat() == Fmt_D32FS8) {
          if(m_Layers == -1) {
            glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0);
            glFramebufferTexture(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,texture->GetTexId(),0);
          } else {
            assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetDepthTexture(): bad m_Layers number");
            glFramebufferTextureLayer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0,m_Layers);
            glFramebufferTextureLayer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,texture->GetTexId(),0,m_Layers);
          }
        } else {
          k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unsupported format .");
        }
      }
      else {
        k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unknown texture type .");
      }
    }
    else {
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferId);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBufferId);
    }
  }

  else if(m_Enabled) {
    if(texture && (texture->GetFlags() & Multisample_Mask) ) {
      // 2d textures
      if(texture->GetType() == Tex_2D) {
        if(texture->GetFormat() == Fmt_D16 || texture->GetFormat() == Fmt_D24 || texture->GetFormat() == Fmt_D32F) {
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->GetTexType(), texture->GetTexId(),0);
          glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBufferId);
        } else if(texture->GetFormat() == Fmt_D24S8 || texture->GetFormat() == Fmt_D32FS8) {
          glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, texture->GetTexType(), texture->GetTexId(),0);
          glFramebufferTexture2D(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT, texture->GetTexType(), texture->GetTexId(),0);
        } else {
          k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unsupported format .");
        }
      }
      // 2d array textures
      else if( texture->GetType() == Tex_2DArray ) {
        if(texture->GetFormat() == Fmt_D16 || texture->GetFormat() == Fmt_D24 || texture->GetFormat() == Fmt_D32F) {
          if(m_Layers == -1) {
            glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0);
          } else {
            assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetDepthTexture(): bad m_Layers number");
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->GetTexId(), 0, m_Layers);
          }
          glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBufferId);
        } else if(texture->GetFormat() == Fmt_D24S8 || texture->GetFormat() == Fmt_D32FS8) {
          if(m_Layers == -1) {
            glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0);
            glFramebufferTexture(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,texture->GetTexId(),0);
          } else {
            assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetDepthTexture(): bad m_Layers number");
            glFramebufferTextureLayer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture->GetTexId(),0,m_Layers);
            glFramebufferTextureLayer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,texture->GetTexId(),0,m_Layers);
          }
        } else {
          k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unsupported format .");
        }
      }
      else {
        k3dLog::Error("kGLFrameBuffer::SetDepthTexture(): unknown texture type .");
      }
    }

    // render buffer
    else {
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferId);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_StencilBufferId);
    }
  }
}

void kGLFrameBuffer::SetColorTexture(kGLTexture *texture, uint32 colAttId)
{
  assert(colAttId >= 0 && colAttId < NUM_RT_TEX && "kGLFrameBuffer::SetColorTexture(): bad texture number");

  if( texture && !texture->IsColorFormat() ) {
    k3dLog::Error("kGLFrameBuffer::SetColorTexture(): unsupported format.");
    return;
  }

  m_ColorTextures[colAttId] = texture;

  if(m_Enabled && m_Samples == 0) {
    if(texture) {
      if((texture->GetFlags() & Filter_Bilinear) || (texture->GetFlags() & Filter_Trilinear)) {
        texture->Bind();
        glGenerateMipmap(texture->GetTexType());
        texture->UnBind();
      }

      if(texture->GetType() == Tex_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexType(),texture->GetTexId(), 0);
      }
      else if(texture->GetType() == Tex_3D) {
        if( -1 == m_Layers ) {
          glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexId(), 0);
        } else {
          assert(m_Layers >= 0 && m_Layers < texture->GetDepth() && "kGLFrameBuffer::SetColorTexture(): bad layer number");
          glFramebufferTexture3D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexType(), texture->GetTexId(), 0, m_Layers);
        }
      }
      else if(texture->GetType() == Tex_Cube) {
        if(-1 == m_Layers) {
          glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexId(), 0);
        } else {
          assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetColorTexture(): bad layer number");
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_Layers, texture->GetTexId(), 0);
        }
      }
      else if(texture->GetType() == Tex_2DArray) {
        if(m_Layers == -1) {
          glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexId(), 0);
        } else {
          assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetColorTexture(): bad layer number");
          glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexId(), 0, m_Layers);
        }
      }
      else {
        k3dLog::Error("kGLFrameBuffer::SetColorTexture(): unknown texture type .");
      }
    }
    else {
      if(colAttId == 0)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBufferId);
      else
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, GL_TEXTURE_2D, 0, 0);
    }
  }
  else if(m_Enabled) {
    if(texture && (texture->GetFlags() & Multisample_Mask)) {
      if(texture->GetType() == Tex_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexType(), texture->GetTexId(), 0);
      }
      else if(texture->GetType() == Tex_2DArray) {
        if(m_Layers == -1) {
          glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexId(), 0);
        } else {
          assert(m_Layers >= 0 && m_Layers < texture->GetLayers() && "kGLFrameBuffer::SetColorTexture(): bad layer number");
          glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, texture->GetTexId(), 0, m_Layers);
        }
      }
      else {
        k3dLog::Error( "kGLFrameBuffer::SetColorTexture(): unknown texture type ." );
      }
    }

    else {
      if(colAttId == 0)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorBufferId);
      else
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colAttId, GL_TEXTURE_2D, 0, 0);
    }
  }
}

bool kGLFrameBuffer::IsCompleted()
{
  if(m_Enabled) {
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      return (status == GL_FRAMEBUFFER_COMPLETE);
  }

  m_Enabled = 1;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_FrameBufferPreviousId);
  glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);

  m_NumTextures = 0;
  if(m_ColorBufferId || m_ColorTextures[0]) {
      m_NumTextures = 1;
  }
  for(int i = 1; i < NUM_RT_TEX; i++) {
      if(m_ColorTextures[i]) m_NumTextures = i + 1;
  }

  for(int i = 0; i < m_NumTextures; i++) {
      SetColorTexture(m_ColorTextures[i], i);
  }
  SetDepthTexture(m_DepthTexture);

  if(m_NumTextures == 0) {
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
  } else {
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
      glReadBuffer(GL_COLOR_ATTACHMENT0);
  }

  m_Enabled = 0;
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferPreviousId);
  return (status == GL_FRAMEBUFFER_COMPLETE);
}

void kGLFrameBuffer::CheckFrameBuffer()
{
  switch( glCheckFramebufferStatus(GL_FRAMEBUFFER) ) {
  case GL_FRAMEBUFFER_COMPLETE:
    return;
  case GL_FRAMEBUFFER_UNSUPPORTED:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: unsupported\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: incomplete attachment\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: incomplete missing attachment\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: incomplete draw buffer\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: incomplete read buffer\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: incomplete multisample\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: incomplete m_Layers targets !");
    break;
  default:
    k3dLog::Error("kGLFrameBuffer::CheckFrameBuffer: failed Unknown !");
    break;
  }
  kDebug("kGLFrameBuffer::CheckFrameBuffer Error Generated ! %s\n", glewGetErrorString( glGetError() ) );
}

void kGLFrameBuffer::Enable()
{
  assert(m_Enabled == 0 && "kGLFrameBuffer::Enable(): is already enabled");

  m_Enabled = 1;
  glGetIntegerv(GL_VIEWPORT, m_Viewport);
//  multisample = GLExt::getMultisample();

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_FrameBufferPreviousId);
  glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);

  m_NumTextures = 0;
  if(m_ColorBufferId || m_ColorTextures[0]) {
      m_NumTextures = 1;
  }
  for(int i = 1; i < NUM_RT_TEX; i++) {
      if(m_ColorTextures[i]) m_NumTextures = i + 1;
  }

  for(int i = 0; i < m_NumTextures; i++) {
      SetColorTexture( m_ColorTextures[i], i );
  }
  SetDepthTexture(m_DepthTexture);

  if(m_NumTextures == 0) {
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
  } else {
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
      glReadBuffer(GL_COLOR_ATTACHMENT0);
  }

  if(m_NumTextures > 1) {
      int num_draw_buffers = 0;
      GLuint draw_buffers[NUM_RT_TEX];
      if( m_ColorBufferId || m_ColorTextures[0]) {
          draw_buffers[num_draw_buffers] = GL_COLOR_ATTACHMENT0 + num_draw_buffers;
          num_draw_buffers = 1;
      }
      for(int i = 1; i < m_NumTextures; i++) {
          if(m_ColorTextures[i]) {
              draw_buffers[num_draw_buffers] = GL_COLOR_ATTACHMENT0 + num_draw_buffers;
              num_draw_buffers++;
          }
      }
      glDrawBuffers(num_draw_buffers, draw_buffers);
  }

  CheckFrameBuffer();

  if( m_ColorTextures[0]) glViewport(0,0, m_ColorTextures[0]->GetWidth(), m_ColorTextures[0]->GetHeight());
  else glViewport(0, 0, m_Width, m_Height);
//  GLExt::setMultisample(samples);
}

void kGLFrameBuffer::Disable()
{
  assert(m_Enabled && "kGLFrameBuffer::Disable(): is not enabled");

  if(m_NumTextures > 1) {
      GLuint draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
      glDrawBuffers(1,draw_buffers);
  }

  m_Enabled = 0;
  glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferPreviousId);
  glViewport(m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3]);
//  GLExt::setMultisample(multisample);
}

int kGLFrameBuffer::GetWidth() const
{
  return m_Width;
}

int kGLFrameBuffer::GetHeight() const
{
  return m_Height;
}

int kGLFrameBuffer::GetFlags() const
{
  return m_Flags;
}

bool kGLFrameBuffer::IsEnabled() const
{
	return m_Enabled;
}

void kGLFrameBuffer::Flush()
{
	assert(m_Enabled && "kGLFrameBuffer::Flush(): is not enabled !");
	if (m_Samples != 0) {
		GLuint mask = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, m_BlitDrawBuffer);
		if (m_ColorTextures[0] && (m_ColorTextures[0]->GetFlags() & Multisample_Mask) == 0) {
			if ((m_ColorTextures[0]->GetFlags() & Filter_Bilinear) || (m_ColorTextures[0]->GetFlags() & Filter_Trilinear) ) {
				m_ColorTextures[0]->Bind(0);
				glGenerateMipmap(m_ColorTextures[0]->GetTexType());
				m_ColorTextures[0]->UnBind();
			}

			if (m_ColorTextures[0]->GetType() == Tex_2D) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorTextures[0]->GetTexType(), m_ColorTextures[0]->GetTexId(), 0);
				mask |= GL_COLOR_BUFFER_BIT;
			}
			else if (m_ColorTextures[0]->GetType() == Tex_3D) {
				assert(m_Layers >= 0 && m_Layers < m_ColorTextures[0]->GetDepth() && "kGLFrameBuffer::flush(): bad layer number");
				glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorTextures[0]->GetTexType(), m_ColorTextures[0]->GetTexId(), 0, m_Layers);
				mask |= GL_COLOR_BUFFER_BIT;
			}
			else if (m_ColorTextures[0]->GetType() == Tex_Cube ) {
				assert(m_Layers >= 0 && m_Layers < 6 && "kGLFrameBuffer::Flush(): bad layer number");
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_Layers, m_ColorTextures[0]->GetTexId(), 0);
				mask |= GL_COLOR_BUFFER_BIT;
			}
			else if (m_ColorTextures[0]->GetType() == Tex_2DArray) {
				assert(m_Layers >= 0 && m_Layers < m_ColorTextures[0]->GetLayers() && "GLFrameBuffer::flush(): bad layer number");
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorTextures[0]->GetTexId(), 0, m_Layers);
			}
			else {
				k3dLog::Error("kGLFrameBuffer::Flush(): unknown color texture type .");
			}
		}
		else {
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
		}


		if (m_DepthTexture && (m_DepthTexture->GetFlags() & Multisample_Mask) == 0) {
			if (m_DepthTexture->GetType() == Tex_2D) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture->GetTexType(), m_DepthTexture->GetTexId(), 0);
				mask |= GL_DEPTH_BUFFER_BIT;
			}
			else if ( m_ColorTextures[0]->GetType() == Tex_Cube ) {
				assert(m_Layers >= 0 && m_Layers < 6 && "kGLFrameBuffer::Flush(): bad layer number");
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_Layers, m_ColorTextures[0]->GetTexId(), 0);
				mask |= GL_DEPTH_BUFFER_BIT;
			}
			else if (m_DepthTexture->GetType() == Tex_2DArray) {
				assert(m_Layers >= 0 && m_Layers < m_DepthTexture->GetLayers() && "GLFrameBuffer::flush(): bad layer number");
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture->GetTexId(), 0, m_Layers);
				mask |= GL_DEPTH_BUFFER_BIT;
			}
			else {
				k3dLog::Error("kGLFrameBuffer::Flush(): unknown depth texture type .");
			}
		}
		else {
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
		}

		// copy read buffer to draw buffer
		if (mask) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBufferId);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BlitDrawBuffer);
			glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, m_Width, m_Height, mask, GL_NEAREST);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferId);
	}
}
