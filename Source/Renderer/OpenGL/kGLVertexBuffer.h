#pragma once
#include "kGLBuffer.h"

struct VertexAttrib {
  uint32 location; // 0,1
  uint32 nComponents; // 3 / 2
  uint32 componentType; // GL_FLOAT...
  uint32 normalized; //GL_TRUE
  uint32 componentSize; // Stride
};

class kGLVertexBuffer : public kGLBuffer {
public:
  kGLVertexBuffer( const BufferDesc vboDesc = BufferDesc::VertexBufferDefault() );
  ~kGLVertexBuffer();
};


class kGLIndexBuffer : public kGLBuffer {
public:
  kGLIndexBuffer(const BufferDesc iboDesc = BufferDesc::IndexBufferDefault() );
  ~kGLIndexBuffer();
};
