#include "Kaleido3D.h"
#include "OGLBuffer.h"
#include <GL/glew.h>

namespace k3d {
  OGLBuffer::OGLBuffer()
  {
    ::glGenBuffers( 1, &m_BufId );
  }

  OGLBuffer::~OGLBuffer()
  {
    if ( ::glIsBuffer( m_BufId ) )
    {
      ::glDeleteBuffers( 1, &m_BufId );
    }
  }

  void OGLBuffer::Allocate( OGLBufferDescriptor const & descriptor )
  {
    if ( glIsBuffer( m_BufId ) ) {
      glBindBuffer( descriptor.Target, m_BufId );
      glBufferData( descriptor.Target, descriptor.Size, descriptor.Data, descriptor.Usage );
      glBindBuffer( descriptor.Target, 0 );
      //
      // glNamedBufferStorageEXT( m_BufId, descriptor.Size, descriptor.Data, flags);
    }
  }

  void OGLBuffer::MakeResident()
  {
    if ( glIsBuffer(m_BufId) )
    {
      glGetNamedBufferParameterui64vNV(m_BufId, GL_BUFFER_GPU_ADDRESS_NV, &m_BufAddr );
      glMakeNamedBufferResidentNV(m_BufId, GL_READ_ONLY );
    }
  }

}