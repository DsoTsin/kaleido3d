#include "Kaleido3D.h"
#include "OGLBuffer.h"
#include "CommonGL.hpp"
#include <GL/glew.h>

namespace k3d {
  OGLBuffer::OGLBuffer()
  {
    ::glGenBuffers( 1, &m_BufId );
  }

  OGLBuffer::~OGLBuffer()
  {
    Release();
  }

  void OGLBuffer::Allocate( OGLBufferDescriptor const & descriptor )
  {
    if ( glIsBuffer( m_BufId ) ) {
      if ( !GLInitializer::BindlessSupported() ) {
        glBindBuffer( descriptor.Target, m_BufId );
        glBufferData( descriptor.Target, descriptor.Size, descriptor.Data, descriptor.Usage );
        glBindBuffer( descriptor.Target, 0 );
      }
      else 
      {
        // bindless buffer storage
        glNamedBufferStorageEXT( m_BufId, descriptor.Size, descriptor.Data, descriptor.Flags);
      }
    }
  }

  void OGLBuffer::Release() {
    if ( ::glIsBuffer( m_BufId ) )
    {
      if ( ::glIsNamedBufferResidentNV( m_BufId ) )
      {
        ::glMakeBufferNonResidentNV( m_BufId );
      }
      glDeleteBuffers( 1, &m_BufId );
    }
  }

  void OGLBuffer::MakeResident()
  {
    if ( ::glIsBuffer(m_BufId) )
    {
      glGetNamedBufferParameterui64vNV(m_BufId, GL_BUFFER_GPU_ADDRESS_NV, &m_BufAddr );
      glMakeNamedBufferResidentNV( m_BufId, GL_READ_ONLY );
    }
  }

}