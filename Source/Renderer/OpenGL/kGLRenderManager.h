#include <Renderer/k3dRenderManager.h>
#include "kGLResourceManager.h"
///Low level
class kGLRenderManager : public k3dRenderManager, public kGLResouceManager {
public:
  kGLRenderManager();
  ~kGLRenderManager();

  void  Init();
  void  Shutdown();

  virtual void CreateContext(SharedWindowPtr &winptr );

  virtual std::shared_ptr<class k3dShader>    FindShader(const k3dString & shaderName) override;
  virtual std::shared_ptr<class k3dTexture>   FindTexture(const k3dString & texName) override;

  // called by Render
  virtual class k3dRenderTexture*             MakeRenderTexture() override;

  // called by AssetManager
  virtual std::shared_ptr<class k3dShader>    MakeShader() override;
  virtual std::shared_ptr<class k3dTexture>   MakeTexture() override;


};
