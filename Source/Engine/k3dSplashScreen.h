#pragma once
//#include <Renderer/k3dRenderManager.h>

class k3dSplashScreen {
public:
  k3dSplashScreen();
  virtual ~k3dSplashScreen();
  bool LoadSplashMovie(const char *movieFile);
  virtual void Render();

protected:
  static float*     sScreenQuad;
  class k3dTexture* mSplashTexture;
  class k3dImage*   mSplashImage;
};
