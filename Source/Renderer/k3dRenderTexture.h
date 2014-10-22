#pragma once

#include "RenderEnums.h"

class k3dRenderTexture {
public:

  k3dRenderTexture();
  virtual ~k3dRenderTexture();

  virtual void Clear() = 0;

  virtual int Create2DRT(int width,int height,int flags = 0) = 0;
  virtual int Create3DRT(int width,int height,int depth,int flags = 0) = 0;
  virtual int CreateCubeRT(int width,int height,int flags = 0) = 0;
  virtual int Create2DArrayRT(int width,int height,int num_layers,int flags = 0) = 0;

  virtual void Release() = 0;

  virtual bool IsCompleted() const = 0;
  virtual void Bind() = 0;
  virtual void UnBind() = 0;
	virtual void Enable() = 0;
	virtual void Disable() = 0;
	virtual void Flush() = 0;

  int GetWidth() const { return m_Width; }
  int GetHeight() const { return m_Height; }
  int GetDepth() const { return m_Depth; }
  int GetLayers() const { return m_Layers; }

  int GetFlags() const { return m_RTFlags; }

  int GetMultiSamples() const {
    if(m_RTFlags & Multisample_2) return 2;
    if(m_RTFlags & Multisample_4) return 4;
    if(m_RTFlags & Multisample_8) return 8;
    return 0;
  }

protected:

  int m_RTType;
  int m_RTFlags;

  int m_Width;
  int m_Height;
  int m_Depth;

  int m_Layers;

};
