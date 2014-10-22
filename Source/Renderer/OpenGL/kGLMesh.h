#pragma once
#include <Renderer/k3dRenderMesh.h>
#include "tGLBuffer.h"
#include "kGLVertexBuffer.h"

class kGLMesh : public k3dRenderMesh {

  public:
    kGLMesh();
    ~kGLMesh();

};

// light weight and static
struct VertexArray
{
public:

  VertexArray();
  ~VertexArray();
  template<typename T>
  void Add(		//typename const VertexBuffer<T>::Buffer& _buffer,
    const T& 			_buffer,
    GLint    			_location,
    int      			_nComponents,
    GLenum   			_componentType,
    bool     			_normalize = false,
    int	 				_offset = 0 );

  // Regular drawing functions
  void Draw( GLenum				_primitiveType,
    const IndexBuffer&	_buffer ) const;

  void Draw( GLenum				_primitiveType,
    const IndexBuffer&	_buffer,
    int					_count,
    int					_first ) const;

  void Draw( GLenum				_primitiveType,
    int					_count,
    int					_first = 0 ) const;

  // Instanced drawing functions
  void Draw( GLenum 				_primitiveType,
    int 				_count,
    int 				_first,
    int 				_primCount ) const;

  void Draw( GLenum				_primitiveType,
    const IndirectArrayBuffer& _indirectBuffer ) const;

  GLuint 			id;
};

template<typename T>
void VertexArray::Add( const T& _buffer, GLint _location, int _nComponents, GLenum _componentType, bool _normalize, int	_offset )
{
  glBindVertexArray( id );
  glBindBuffer( GL_ARRAY_BUFFER, _buffer.id );
  glVertexAttribPointer( _location, _nComponents, _componentType, _normalize, sizeof(typename T::DataType), KGLBUFFER_OFFSET( _offset ) );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glEnableVertexAttribArray( _location );
  glBindVertexArray( 0 );
}
