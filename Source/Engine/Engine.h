#pragma once
#include <Config/Prerequisities.h>
#include <KTL/Singleton.h>
#include <Core/k3dApplication.h>
#include <Core/k3dWindow.h>
#include <Renderer/k3dRender.h>
#include <Engine/k3dController.h>
#include <Engine/k3dSceneManager.h>


class k3dRenderManager;

struct kEngineGlobals {
  int                   WindowWidth;
  int                   WindowHeight;
  k3dString             WindowName;
  k3dString             LogPath;
  k3dString             DataPath;
  k3dString             DefaultLevel;


  SharedWindowPtr       Window;
  SharedAppPtr          Application;
  SharedControllerPtr   Controller;
  SharedRenderPtr       Render;
  SharedSceneManagerPtr Scene;


  kEngineGlobals() {
    WindowWidth = 0;
    WindowHeight= 0;
    Window      = nullptr;
    Application = nullptr;
    Controller  = nullptr;
    Render      = nullptr;
    Scene       = nullptr;
  }
};

extern kEngineGlobals engine;

class kEngine : public Singleton<kEngine> {
public:
  kEngine();
  ~kEngine();

  int   LoadConfig(const char* fileEngineConf);
  void  Init();
  void  AddSplashScreen(float time_length);
  bool  LoadLevel(const char * levelFileName);

  void  Main();
  void  Shutdown();

private:

  k3dRenderManager *m_RendererManagerPtr;
};
