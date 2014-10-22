#pragma once

class k3dGPUBuffer {
public:
  /// \brief BufferType enum
  ///         VertexBuffer, UniformBuffer...
  ///
  enum BufferType {
    VertexBuffer,
    ElementBuffer,
    IndirectBuffer,
    UniformBuffer,
    ShaderStorageBuffer,
    TextureBuffer
  };

  enum BufferFlag {
    StaticDraw,
    DynamicDraw,
    StreamDraw
  };

  enum BufferAccessFlag {
    MapRead = 1,
    MapWrite = 1<<1,
    MapPersistent = 1<<2,
    MapCoherent = 1<<3,
    MapDynamicStorage = 1<<4,
    MapClientStorage = 1<<5,
    MapClientBarrier = 1<<6
  };

  virtual ~k3dGPUBuffer() {}

  virtual void    Allocate(uint32 size) = 0;
  virtual void    Release() = 0;
  virtual void    Bind() = 0;
  virtual void*   MapRange(uint32 start, uint32 offset) = 0;
  virtual void    UnMap() = 0;
};
