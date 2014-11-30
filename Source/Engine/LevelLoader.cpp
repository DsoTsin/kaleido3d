#include "k3dLevelLoader.h"
#include <json/json.h>
#include <Core/k3dFile.h>
#include <Core/k3dDbg.h>
#include <Core/k3dJsonObject.h>
#include "k3dSceneManager.h"
#include "k3dEngine.h"
#include "k3dFreeCamera.h"
#include "k3dTargetCamera.h"
#include "k3dFollowCamera.h"
#include <Entity/k3dVehicleController.h>

#define LOG_WHEN( expression, condition ) \
  if(!(condition)) {\
    kDebug( (expression ## "in file %s line %d\n"), __FILE__, __LINE__ ) ;\
  }

static bool createController(k3dString & controllerName);
static bool createCamera( k3dString & cameraType, kMath::Vec3f & position, kMath::Vec3f & rotation );
static bool createTerrain( int res_w, int res_h );
static bool createPlayer(Json::Value & player, k3dString & playerType );
static bool createLights( Json::Value & lights );

k3dLevelLoader::k3dLevelLoader(std::shared_ptr<k3dSceneManager> scene)
  : m_Scene( scene )
{
}

k3dLevelLoader::~k3dLevelLoader()
{
}

bool k3dLevelLoader::LoadLevel(const char *levelFileName)
{
  k3dFile levelFile(levelFileName);
  if(!levelFile.Open(IORead)) {
    kDebug("k3dLevelLoader::LoadLevel Error cannot find level file (%s).\n", levelFileName);
    return false;
  }
  int sizeFile = (int)levelFile.GetSize();
  char *levelDesStr = new char[sizeFile+1];
  levelFile.Read(levelDesStr, sizeFile);
  levelDesStr[sizeFile] = 0;
  levelFile.Close();

  Json::Reader levelReader;
  Json::Value  levelRoot;
  if( !levelReader.parse(k3dString(levelDesStr), levelRoot, false) ) {
    kDebug("k3dLevelLoader::LoadLevel Cannot parse level description file.\n");
    return false;
  }

  Json::Value & level = levelRoot["Level"];
  kDebug("Level Name : %s.\n", level["Name"].asString().c_str());

  //! Set Up Controller
  k3dString & controller = level["Controller"].asString();
  LOG_WHEN("Failed to create engine controller!\n", createController( controller ) );

  //! Set Up Camera
  Json::Value & camera = level["Camera"];
  k3dString & cameraType = camera["Type"].asString();
  kMath::Vec3f & cameraPosition = JVtoVec3f( camera["Position"] );
  kMath::Vec3f & cameraRotation = JVtoVec3f( camera["Rotation"] );
  LOG_WHEN("Unknown Camera type. failed to create camera !\n", createCamera(cameraType, cameraPosition, cameraRotation) );

  //! Set Up Player
  Json::Value & player = level["Player"];
  k3dString & playerType = player["Type"].asString();
  LOG_WHEN("failed to create player.", createPlayer(player, playerType));

  //! Set Up Terrain
  Json::Value & terrain = level["Terrain"];
  int terrainWidth = terrain["Width"].asInt();
  int terrainHeight = terrain["Height"].asInt();
  LOG_WHEN("failed to create terrain.", createTerrain( terrainWidth, terrainHeight ));

  //! Set Up Lights
  Json::Value & lightsArray = level["Lights"];
  LOG_WHEN("failed to createLights.", createLights(lightsArray) );

  Json::Value & staticObjectsArray = level["StaticObjects"];
  if( !staticObjectsArray.isArray() ) {
    kDebug("k3dLevelLoader::LoadLevel Error cannot find static objects array .\n");
    return false;
  }

  return true;
}

static bool createController(k3dString &controllerName)
{
  if(controllerName == "VehicleController") {
    engine.Controller = SharedControllerPtr( new kVehicleController );
    std::static_pointer_cast<kVehicleController>( engine.Controller ) -> InitController();
    return true;
  }

  else
    return false;
}

static bool createCamera(k3dString &cameraType, kMath::Vec3f &position, kMath::Vec3f &rotation)
{
  std::shared_ptr<k3dCamera> spCamera;

  if( cameraType == "FollowCamera" ) {
    std::shared_ptr<k3dFollowCamera> followCamera( new k3dFollowCamera );
    spCamera = followCamera;
  }
  else if( cameraType == "FreeCamera" ) {
    std::shared_ptr<kFreeCamera> freeCamera( new kFreeCamera );
    freeCamera->SetSpeed( 0.05f );
    spCamera = freeCamera;
  }
  else
    return false;

  spCamera->SetPosition( position );
  spCamera->Rotate( rotation[0], rotation[1], rotation[2] );
  spCamera->SetFOV(45.f);
  spCamera->SetProjection(45.f, 1.0f*engine.WindowWidth/engine.WindowHeight, 0.01f, 1000.f);
  spCamera->Update();
  engine.Controller->SetCamera( spCamera );
  return true;
}

static bool createTerrain(int res_w, int res_h)
{
  return false;
}

static bool createPlayer(Json::Value & player, k3dString & playerType ) {
  kMath::Vec3f & playerPosition = JVtoVec3f( player["Position"] );
  kMath::Vec3f & playerRotation = JVtoVec3f( player["Rotation"] );
  if( playerType == "Vehicle" ) {
//    m_Scene
  }
  return false;
}

static bool createLights(Json::Value &lights) {
  if( !lights.isArray() ) {
    kDebug("createLights Error. lights isn't an array... in (%s, line %d)\n", __FILE__, __LINE__);
    return false;
  }

  for(Json::ValueIterator lightIter = lights.begin(); lightIter != lights.end(); ++lightIter ) {
    Json::Value & light = (*lightIter);
    k3dString lightName = light["Name"].asString();
    k3dString lightType = light["Type"].asString();
    kMath::Vec3f lightPosition = JVtoVec3f( light["Position"] );
    kMath::Vec3f lightRotation = JVtoVec3f( light["Rotation"] );

  }

  return true;
}
