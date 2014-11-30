#include "k3dEngine.h"
#include <fstream>
#include <json/json.h>
#include <Core/k3dLog.h>
#include <Core/k3dDbg.h>
#include <Core/k3dInputEvent.h>
#include <Engine/k3dAssetManager.h>
#include <Engine/k3dLevelLoader.h>
#include <Renderer/OpenGL/kGLRender.h>
#include <Physics/kPsEngine.h>
#include <Audio/kAuEngine.h>
#include <Entity/k3dVehicleController.h>


#if defined(USE_GLFW)
#include <Platform/GLFW/kFWindow.h>
#include <Platform/GLFW/kFApplication.h>
#endif

using namespace std;

kEngineGlobals engine;

kEngine::kEngine()
{
}

kEngine::~kEngine()
{

}

int kEngine::LoadConfig(const char *fileEngineConf)
{
  ifstream ifs;
  ifs.open(fileEngineConf);
  assert(ifs.is_open());

  Json::Reader confLoader;
  Json::Value engineConf;
  if (!confLoader.parse(ifs, engineConf, false))
  {
    ifs.close();
    return -1;
  }
  ifs.close();

  engine.DataPath     = engineConf["DataPath"].asString();
  engine.LogPath      = engineConf["Log"].asString();
  engine.WindowWidth  = engineConf["ResolutionWidth"].asInt();
  engine.WindowHeight = engineConf["ResolutionHeight"].asInt();
  engine.DefaultLevel = engineConf["DefaultLevel"].asString();

  return 1;
}

void kEngine::Init()
{
  k3dLog::InitLogFile(engine.LogPath.c_str());
  k3dLog::Message("Engine::Init.....");
#if defined(USE_GLFW)
  SharedWindowDefPtr winDef = SharedWindowDefPtr(new kWindowDefinition);
  (*winDef).Width           = engine.WindowWidth;
  (*winDef).Height          = engine.WindowHeight;
  (*winDef).Title           = "Launcher";

  engine.Application    = SharedAppPtr(new kFApplication);
  engine.Application    ->InitApplication();
  engine.Window         = engine.Application->MakeWindow();

  engine.Application->InitializeWindow(engine.Window, winDef, true);
#endif

  k3dAssetManager & assetMan = k3dAssetManager::Get();
  assetMan.Init();
  assetMan.AddSearchPath( engine.DataPath.c_str() );

  //![5] Init RenderManager
  engine.Render = SharedRenderPtr( new kGLRender );
  engine.Render->Init( engine.Window );

//  kVehicleController * vehicle = new kVehicleController;
//  vehicle->InitController();
//  engine.Controller = SharedControllerPtr(vehicle);
  if( !LoadLevel( (engine.DataPath + engine.DefaultLevel + ".level").c_str() ) ) {
    kDebug("Level(%s) failed to Load ! \n", engine.DefaultLevel.c_str() );
    k3dLog::Error( "Level(%s) failed to Load ! \n", engine.DefaultLevel.c_str() );
  }


  kPsEngine::Get().InitPhysics();
}

bool kEngine::LoadLevel(const char *levelFileName)
{
  // scene manager must be initialized first
  k3dLevelLoader loader( engine.Scene );
  return loader.LoadLevel(levelFileName);
}

void kEngine::Main()
{
  bool gotMsg = false;
  while (engine.Application->QueryMessage(gotMsg))
  {
    engine.Application->PumpMessages(0.0f);
    engine.Render->Render();
    engine.Window->SwapBuffers();
  }
}

void kEngine::Shutdown()
{
  engine.Render->Shutdown();
  kPsEngine::Get().ExitPhysics();
  k3dAssetManager::Get().Shutdown();
  k3dLog::Message("Engine::Shutdown.....");
  engine.Application->DestroyApplication();
  k3dLog::CloseLog();
}
