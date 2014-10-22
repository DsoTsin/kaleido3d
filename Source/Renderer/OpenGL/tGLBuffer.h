#ifndef __tGLBuffer_h__
#define __tGLBuffer_h__
#pragma once 

#include <Config/Prerequisities.h>
#include <Math/kMath.hpp>
#include <GL/glew.h>
#include <Renderer/k3dGPUBuffer.h>

#define KGLBUFFER_OFFSET(i) 	((char *)NULL + (i))

template<GLenum B, typename T>
class kTGLBuffer : public k3dGPUBuffer
{
public:
  // Buffer Element Type
  typedef T 		DataType;
  // Buffer Type
  const GLenum 	BufferType;

  ~kTGLBuffer();

  explicit          kTGLBuffer();
  explicit          kTGLBuffer( int _nElements, uint32 _update = GL_DYNAMIC_DRAW );

  void 	 		    Allocate( uint32 _nElements );
  void 			    Allocate( uint32 _nElements, uint32 _update );
  void              Bind();

  void*             MapRange(uint32 start, uint32 offset) { return nullptr; }

  void              Release() { if(glIsBuffer(id)) { glDeleteBuffers( 1, &id ); } }

  inline T* 		Lock( uint32 _access = GL_READ_WRITE );
  void              UnMap();
  inline void       Fill( T* _data, int _count );

private:
  kTGLBuffer( kTGLBuffer<B, T>& );
  kTGLBuffer<B, T>& operator=(kTGLBuffer<B, T>&) = delete;
public:
  // this is the buffer handle
  GLuint			id;
  // update can be GL_STREAM_DRAW, GL_STATIC_DRAW, GL_DYNAMIC_DRAW
  GLenum 			update;
  GLsizei			count;
  bool              lock;
};

template<GLenum B, typename T>
kTGLBuffer<B, T>::kTGLBuffer(int _count, uint32 _update ) : BufferType( B ), update( _update ), count( _count ), lock( false )
{
  glGenBuffers( 1, &id );
  glBindBuffer( B, id );
  glBindBuffer( B, 0 );
  Allocate( count, update );
}

template<GLenum B, typename T>
kTGLBuffer<B, T>::kTGLBuffer( ) : BufferType( B ), update( GL_DYNAMIC_DRAW ), count( 0 ), lock( false )
{
  glGenBuffers( 1, &id );
  glBindBuffer( B, id );
  glBindBuffer( B, 0 );
}

template<GLenum B, typename T>
void kTGLBuffer<B, T>::Allocate(uint32 _count )
{
  Allocate( _count, update );
}

template<GLenum B, typename T>
void kTGLBuffer<B, T>::Allocate(uint32 _count, uint32 _update )
{
  assert( _count > 0 );
  count = _count;
  update = _update;

  glBindBuffer( B, id );
  glBufferData( B, count*sizeof(T), NULL, update );
}

template<GLenum B, typename T>
kTGLBuffer<B, T>::~kTGLBuffer()
{
  Release();
}

template<GLenum B, typename T>
T* kTGLBuffer<B, T>::Lock( uint32 _access )
{
  assert( !lock );
  glBindBuffer( B, id );
  void* p = glMapBuffer( B, _access );
  assert( p != NULL );
  lock = true;
  return (T*)p;
}

template<GLenum B, typename T>
void kTGLBuffer<B, T>::UnMap()
{
  assert( lock );
  glBindBuffer( B, id );
  glUnmapBuffer( B );
  lock = false;
}

template<GLenum B, typename T>
void kTGLBuffer<B, T>::Fill( T* _data, int _count )
{
  assert( !lock );
  assert( _count <= count );
  glBindBuffer( B, id );
  glBufferData( B, _count*sizeof(T), (void*)_data, update );
}

// Indirect buffer structure 
struct DrawArraysIndirectCommand
{
  GLuint count;
  GLuint primCount;
  GLuint first;
  GLuint reservedMustBeZero;
};

// Indirect indexed buffer structure 
struct DrawElementsIndirectCommand
{
  GLuint count;
  GLuint primCount;
  GLuint firstIndex;
  GLint  baseVertex;
  GLuint reservedMustBeZero;
};

template<class T> struct VertexBuffer
{
  typedef T                               Data;
  typedef kTGLBuffer<GL_ARRAY_BUFFER, T>  Buffer;
};

template<class T> struct UniformBuffer
{
  typedef T                               Data;
  typedef kTGLBuffer<GL_UNIFORM_BUFFER, T> Buffer;
};

template<class T> struct CopyReadBuffer		// GPU -> CPU only
{
  typedef T                                     Data;
  typedef kTGLBuffer<GL_COPY_READ_BUFFER, Data> Buffer;
};

template<class T> struct CopyWriteBuffer	// CPU -> GPU only
{
  typedef T                                       Data;
  typedef kTGLBuffer<GL_COPY_WRITE_BUFFER, Data>  Buffer;
};

template<class T> struct PixelPackBuffer	// GPU -> CPU only
{
  typedef T                                       Data;
  typedef kTGLBuffer<GL_PIXEL_PACK_BUFFER, Data>  Buffer;
};

template<class T> struct PixelUnpackBuffer	// CPU -> GPU only
{
  typedef T                                         Data;
  typedef kTGLBuffer<GL_PIXEL_UNPACK_BUFFER, Data>  Buffer;
};


typedef kTGLBuffer<GL_DRAW_INDIRECT_BUFFER, DrawArraysIndirectCommand>    IndirectArrayBuffer;
typedef kTGLBuffer<GL_DRAW_INDIRECT_BUFFER, DrawElementsIndirectCommand>  IndirectElementBuffer;
typedef kTGLBuffer<GL_ELEMENT_ARRAY_BUFFER, uint32>		                  IndexBuffer;
typedef kTGLBuffer<GL_ATOMIC_COUNTER_BUFFER, uint32>                      AtomicCounterBuffer;

typedef VertexBuffer<float>::Buffer                 VertexBuffer1F;
typedef VertexBuffer<kMath::Vec2f>::Buffer			VertexBuffer2F;
typedef VertexBuffer<kMath::Vec3f>::Buffer			VertexBuffer3F;
typedef VertexBuffer<kMath::Vec4f>::Buffer			VertexBuffer4F;

#endif
