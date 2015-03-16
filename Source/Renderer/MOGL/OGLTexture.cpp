#include "Kaleido3D.h"
#include "OGLTexture.h"
#include <GL/glew.h>

namespace k3d {

  OGLTexture::OGLTexture()
  {
    m_TexId = GL_INVALID_INDEX;
    m_TexAddr = 0;
    ::glGenTextures( 1, &m_TexId );
  }

  void OGLTexture::MakeResident() 
  {
    if ( ::glIsTexture( m_TexId ) )
    {
      m_TexAddr = ::glGetTextureHandleARB( m_TexId );
      ::glMakeTextureHandleResidentARB( m_TexAddr );
    }
  }

  OGLTexture::~OGLTexture()
  {
    if ( ::glIsTexture(m_TexId) )
    {
      ::glDeleteTextures( 1, &m_TexId );
    }
  }

  void OGLTexture::Allocate( OGLTextureDescriptor const & descriptor )
  {
    if ( ::glIsTexture( m_TexId ) ) 
    {
      switch ( descriptor.Target )
      {
      case GL_TEXTURE_1D:
        glBindTexture( GL_TEXTURE_1D, m_TexId );
        glTexStorage1D( GL_TEXTURE_1D, descriptor.Levels, descriptor.InternalFormat, descriptor.Width );
        glBindTexture( GL_TEXTURE_1D, 0 );
        break;
      case GL_TEXTURE_2D:
        glBindTexture( GL_TEXTURE_2D, m_TexId );
        glTexStorage2D( GL_TEXTURE_2D, descriptor.Levels, descriptor.InternalFormat, descriptor.Width, descriptor.Height );
        glBindTexture( GL_TEXTURE_2D, 0 );
        break;
      case GL_TEXTURE_3D:
        glBindTexture( GL_TEXTURE_3D, m_TexId );
        glTexStorage3D( GL_TEXTURE_3D, descriptor.Levels, descriptor.InternalFormat, descriptor.Width, descriptor.Height, descriptor.Depth );
        glBindTexture( GL_TEXTURE_3D, 0 );
        break;
      default:
        break;
      }
    }
  }
}