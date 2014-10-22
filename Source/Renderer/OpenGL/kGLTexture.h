#ifndef __k3dTexture_h__
#define __k3dTexture_h__
#pragma once
#include <Renderer/k3dTexture.h>

class kGLTexture : public k3dTexture {
public:

  kGLTexture();
  virtual ~kGLTexture();

  virtual void Bind(uint32 textureUnit) override;
  void          Bind();
  void          UnBind();

  virtual void Release() override;

  virtual int Create(const k3dImage & image,
      uint32 flag = Default_Flag) override;

  virtual int Create1D(
      int width,
      uint32 format,
      uint32 flag) override;

  virtual int Create2D(
      int width, int height,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int Create3D(
      int width, int height, int depth,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int CreateCube(
      int width, int height,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int Create2DArray(
      int width, int height, int num_layers,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int UpdateFormat();
  virtual void SetFlag(uint32 flag) override;
  virtual void SetPixelAlignment(uint32 align) override;

  uint32 GetTexType() const;
  uint32 GetTexId() const;

protected:

  uint32 m_TexId;
  uint32 m_TexType;
  uint32 m_TexInternalFormat;
};


class kGLImmuTexture : public kGLTexture {
public:

  kGLImmuTexture();
  ~kGLImmuTexture();

  virtual int Create1D(
      int width,
      uint32 format,
      uint32 flag) override;

  virtual int Create2D(
      int width, int height,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int Create3D(
      int width, int height, int depth,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int CreateCube(
      int width, int height,
      uint32 format,
      uint32 flag = Default_Flag) override;

  virtual int Create2DArray(
      int width, int height, int num_layers,
      uint32 format,
      uint32 flag = Default_Flag) override;

};


#endif
