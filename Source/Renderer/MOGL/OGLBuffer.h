#pragma once

namespace k3d {
  struct OGLBufferDescriptor
  {
    uint32 Target;
    uint32 Size;
    void * Data;
    uint32 Usage;
  };
  class OGLBuffer
  {
  public:
    OGLBuffer();
    virtual ~OGLBuffer();

    void Allocate(OGLBufferDescriptor const & decriptor);
    void MakeResident();
  private:
    uint32 m_BufId;
    uint64 m_BufAddr;
  };
}