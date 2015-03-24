#pragma once

namespace k3d {

  struct OGLTextureDescriptor {
    uint32 Target;
    uint32 Levels;
    uint32 InternalFormat;
    uint32 Width;
    uint32 Height;
    uint32 Depth;

    OGLTextureDescriptor() { }
  };

  class OGLTexture
  {
  public:
    OGLTexture();
    virtual ~OGLTexture();

    void Allocate( OGLTextureDescriptor const & descriptor );
    void Release();
    void MakeResident();

  private:
    uint32 m_TexId;
    uint64 m_TexAddr;
  };
}
