#pragma once
#include <Core/k3dApplication.h>

class k3dEngineContext;

/// GPU Side Resource Manager
/// Create StaticMesh, 2DTexture, RenderTarget...
/// Find / Query Resource

class k3dRenderManager
{
public:
  k3dRenderManager();
  virtual ~k3dRenderManager();

  virtual void Init();
  virtual void Shutdown();
  virtual void CreateContext( SharedWindowPtr &winptr );
  virtual void MakeCurrent();
  virtual int GetVideoMemoryUseage();

  virtual std::shared_ptr<class k3dShader>   FindShader(const k3dString & shaderName) = 0;
  virtual std::shared_ptr<class k3dTexture>  FindTexture(const k3dString & texName) = 0;

  virtual class k3dRenderTexture*            MakeRenderTexture() = 0;
  virtual std::shared_ptr<class k3dShader>   MakeShader() = 0;
  virtual std::shared_ptr<class k3dTexture>  MakeTexture() = 0;

protected:

  k3dEngineContext*    m_Context;
};
