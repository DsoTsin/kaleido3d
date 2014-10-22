#pragma once
#include <Renderer/k3dGPUBuffer.h>

struct BufferDesc {
  k3dGPUBuffer::BufferType        BufferType;
  k3dGPUBuffer::BufferFlag        BufferFlag;
  k3dGPUBuffer::BufferAccessFlag  BufferAccessFlag;

  static BufferDesc VertexBufferDefault() {
    static BufferDesc bd = {
      k3dGPUBuffer::VertexBuffer,
      k3dGPUBuffer::StaticDraw,
      k3dGPUBuffer::BufferAccessFlag(k3dGPUBuffer::MapWrite | k3dGPUBuffer::MapRead)
    };
    return bd;
  }

  static BufferDesc IndexBufferDefault() {
    static BufferDesc bd = {
      k3dGPUBuffer::ElementBuffer,
      k3dGPUBuffer::StaticDraw,
      k3dGPUBuffer::BufferAccessFlag(k3dGPUBuffer::MapWrite | k3dGPUBuffer::MapRead)
    };
    return bd;
  }
};

class kGLBuffer : public k3dGPUBuffer {
public:
  explicit kGLBuffer( const BufferDesc & bufferDesc );
  virtual ~kGLBuffer();

  virtual void    Allocate(uint32 size);
  virtual void*   AllocateStorage(uint32 size);

  virtual void    Release() override;
  virtual void    Bind();
  virtual void*   MapRange(uint32 start, uint32 size);
  virtual void    UnMap();

protected:
  uint32            m_BufferType;
  uint32            m_BufferFlag;
  uint32            m_BufferAccessFlag;
  uint32            m_BufferHandle;

private:
  kGLBuffer(const kGLBuffer &);
  kGLBuffer & operator = (const kGLBuffer &);
};
