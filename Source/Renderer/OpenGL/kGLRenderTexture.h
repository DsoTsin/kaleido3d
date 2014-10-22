#pragma once

#include <Renderer/k3dRenderTexture.h>

class kGLRenderTexture : public k3dRenderTexture {
public:
  kGLRenderTexture();
  virtual ~kGLRenderTexture();

  void          Clear() override;

  virtual int   Create2DRT(int width,int height,int flags = 0) override;
  virtual int   Create3DRT(int width,int height,int depth,int flags = 0) override;
  virtual int   CreateCubeRT(int width,int height,int flags = 0) override;
  virtual int   Create2DArrayRT(int width,int height,int num_layers,int flags = 0) override;

  virtual void  Release();

  virtual bool  IsCompleted() const;
  virtual void  Bind();
  virtual void  UnBind();

	virtual void	Enable() override;
	virtual void	Disable() override;
	virtual void  Flush() override;

private:
  int                     create();
  class kGLTexture*       createTexture(int format, int flags);

  class kGLFrameBuffer*   m_FrameBuffer;
  class kGLTexture*       m_ColorTexture;
  class kGLTexture*       m_DepthTexture;


};
