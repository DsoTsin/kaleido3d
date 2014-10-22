#include "kGLBuffer.h"
#include <GL/glew.h>
#include <assert.h>

kGLBuffer::kGLBuffer(const BufferDesc &bufferDesc)
{
  static uint32 bufferTypes[] = {
    GL_ARRAY_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER,
    GL_DRAW_INDIRECT_BUFFER,
    GL_UNIFORM_BUFFER,
    GL_SHADER_STORAGE_BUFFER,
    GL_TEXTURE_BUFFER
  };

  static uint32 bufferFlags[] = {
    GL_STATIC_DRAW,
    GL_DYNAMIC_DRAW,
    GL_STREAM_DRAW
  };

  m_BufferType        = bufferTypes[(int)bufferDesc.BufferType];
  m_BufferFlag        = bufferFlags[(int)bufferDesc.BufferFlag];
  m_BufferAccessFlag  = bufferDesc.BufferAccessFlag;
  m_BufferHandle      = -1;
}

kGLBuffer::~kGLBuffer()
{
}


void kGLBuffer::Allocate(uint32 size)
{
  if(!glIsBuffer(m_BufferHandle)) {
    glGenBuffers(1, &m_BufferHandle);
    glBindBuffer(m_BufferType, m_BufferHandle);
    glBufferData(m_BufferType, size, nullptr, m_BufferFlag);
  }
}

void *kGLBuffer::AllocateStorage(uint32 size)
{
  if(!glIsBuffer(m_BufferHandle)) {
    glGenBuffers(1, &m_BufferHandle);
    glBindBuffer(m_BufferType, m_BufferHandle);
    glBufferStorage(m_BufferType, size, nullptr, m_BufferAccessFlag | MapDynamicStorage);
    void* ptr = glMapBuffer(m_BufferType, m_BufferAccessFlag);
    assert(GL_NO_ERROR==glGetError());
    return ptr;
  } else {
    return nullptr;
  }
}

void kGLBuffer::Release()
{
  if(glIsBuffer(m_BufferHandle)) {
    glDeleteBuffers(1, &m_BufferHandle);
    m_BufferHandle = -1;
  }
}

void kGLBuffer::Bind()
{
  glBindBuffer(m_BufferType, m_BufferHandle);
}

void *kGLBuffer::MapRange(uint32 start, uint32 size)
{
  return glMapBufferRange(m_BufferType, start, size, m_BufferAccessFlag);
}

void kGLBuffer::UnMap()
{
  glUnmapBuffer(m_BufferType);
}
