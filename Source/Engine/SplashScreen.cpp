#include "k3dSplashScreen.h"
#include <Renderer/k3dRenderManager.h>
#include <Engine/k3dEngine.h>

//float * k3dSplashScreen::sScreenQuad = {
//  -1.0f,  1.0f,
//  -1.0f, -1.0f,
//   1.0f, -1.0f,
//   1.0f, -1.0f,
//   1.0f,  1.0f,
//  -1.0f,  1.0f
//};

k3dSplashScreen::k3dSplashScreen()
{

}


k3dSplashScreen::~k3dSplashScreen()
{

}


bool k3dSplashScreen::LoadSplashMovie(const char *movieFile)
{
  k3dRenderManager * renderManager = engine.Render->GetRenderManager();
  renderManager->MakeTexture();
  return true;
}


void k3dSplashScreen::Render()
{

}
