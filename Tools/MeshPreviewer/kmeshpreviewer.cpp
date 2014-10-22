#include <GL/glew.h>
#include <assert.h>
#include <json/json.h>
#include <GLFW/glfw3.h>
#include <GL/glfx.h>
#include <Core/k3dDbg.h>
#include <Core/k3dLog.h>
#include <Core/k3dFile.h>
#include <Core/k3dArchive.h>
#include <Core/k3dMesh.h>
#include <Core/k3dImage.h>
#include <Core/DDSHelper/DDSHelper.h>
#include <Core/k3dMaterial.h>
//#include <Core/
#include <Renderer/OpenGL/kGLTechnique.h>
#include <Renderer/OpenGL/kGLTexture.h>

#include <Physics/kPsEngine.h>

#include <Entity/k3dVehicle.h>

#include <Physics/k3dVehicleManager.h>

#include "Physics/kVehicleRaycast.h"

extern "C" {
__declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
}

#define TEST_MESH "../../Data/Objects/Models/test/test.ksmesh"
#define TEST_MATERIAL "../../Data/Objects/Models/test/test.mat"
#define PHONG_SHADER "../../Shader/BlinnPhong.glfx"

#define CHECK_GL_ERROR \
{\
  GLenum gl_error = glGetError();\
  assert(GL_NO_ERROR==gl_error);\
  if(gl_error!=GL_NO_ERROR) kDebug("Error: %s\n", glewGetErrorString(gl_error));\
  }

GLuint  *g_vao_t = nullptr;
GLuint  g_vbo_t = 0;
GLuint  g_ibo_t = 0;
GLuint  g_nbo_t = 0;
GLuint  g_prog_phong = 0;

GLuint gProj = 0;
GLuint gWorld = 0;
GLuint gView = 0;


kMath::Mat4f modelMatrix;
kMath::Mat4f viewMatrix;
kMath::Mat4f projMatrix;

std::map<k3dString, k3dMesh*>  meshes;
std::map<k3dString, uint32>    vaos;

std::vector<k3dTexture*> textures;
std::vector<k3dMaterial*> materials;

kGLTechnique tech;


PxPhysics*          gPhysics        = nullptr;
k3dVehicleManager*  gVehicleManager  = nullptr;
PxMaterial*         gMaterial       = nullptr;
PxScene*            gScene          = nullptr;
PxRigidStatic*      gGroundPlane    = nullptr;


PxVehicleDrive4W*   gVehicle4W		= nullptr;

struct VehicleDesc
{
    PxF32 chassisMass;
    PxVec3 chassisDims;
    PxVec3 chassisMOI;
    PxVec3 chassisCMOffset;
    PxMaterial* chassisMaterial;
    PxF32 wheelMass;
    PxF32 wheelWidth;
    PxF32 wheelRadius;
    PxF32 wheelMOI;
    PxMaterial* wheelMaterial;
    PxU32 numWheels;
};




PxVehicleKeySmoothingData gKeySmoothingData=
{
    {
        6.0f,	//rise rate eANALOG_INPUT_ACCEL
        6.0f,	//rise rate eANALOG_INPUT_BRAKE
        6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
        2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        10.0f,	//fall rate eANALOG_INPUT_ACCEL
        10.0f,	//fall rate eANALOG_INPUT_BRAKE
        10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE
        5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
        5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

PxVehiclePadSmoothingData gPadSmoothingData=
{
  {
    6.0f,	//rise rate eANALOG_INPUT_ACCEL
    6.0f,	//rise rate eANALOG_INPUT_BRAKE
    6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
    2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
    2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
  },
  {
    10.0f,	//fall rate eANALOG_INPUT_ACCEL
    10.0f,	//fall rate eANALOG_INPUT_BRAKE
    10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE
    5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
    5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
  }
};

PxVehicleDrive4WRawInputData gVehicleInputData;

enum DriveMode
{
    eDRIVE_MODE_ACCEL_FORWARDS=0,
    eDRIVE_MODE_ACCEL_REVERSE,
    eDRIVE_MODE_HARD_TURN_LEFT,
    eDRIVE_MODE_HANDBRAKE_TURN_LEFT,
    eDRIVE_MODE_HARD_TURN_RIGHT,
    eDRIVE_MODE_HANDBRAKE_TURN_RIGHT,
    eDRIVE_MODE_BRAKE,
    eDRIVE_MODE_NONE
};

DriveMode gDriveModeOrder[] =
{
    eDRIVE_MODE_BRAKE,
    eDRIVE_MODE_ACCEL_FORWARDS,
    eDRIVE_MODE_BRAKE,
    eDRIVE_MODE_ACCEL_REVERSE,
    eDRIVE_MODE_BRAKE,
    eDRIVE_MODE_HARD_TURN_LEFT,
    eDRIVE_MODE_BRAKE,
    eDRIVE_MODE_HARD_TURN_RIGHT,
    eDRIVE_MODE_ACCEL_FORWARDS,
    eDRIVE_MODE_HANDBRAKE_TURN_LEFT,
    eDRIVE_MODE_ACCEL_FORWARDS,
    eDRIVE_MODE_HANDBRAKE_TURN_RIGHT,
    eDRIVE_MODE_NONE
};

PxF32					gVehicleModeLifetime = 4.0f;
PxF32					gVehicleModeTimer = 0.0f;
PxU32					gVehicleOrderProgress = 0;
bool					gVehicleOrderComplete = false;
bool					gMimicKeyInputs = false;


//Drivable surface types.
enum
{
    SURFACE_TYPE_TARMAC,
    MAX_NUM_SURFACE_TYPES
};


PxRigidStatic* createDrivablePlane(PxMaterial* material, PxPhysics* physics) {
  //Add a plane to the scene.
  PxRigidStatic* groundPlane = PxCreatePlane(*physics, PxPlane(0,1,0,0), *material);

  //Get the plane shape so we can set query and simulation filter data.
  PxShape* shapes[1];
  groundPlane->getShapes(shapes, 1);

  //Set the query filter data of the ground plane so that the vehicle raycasts can hit the ground.
  physx::PxFilterData qryFilterData;
  setupDrivableSurface(qryFilterData);
  shapes[0]->setQueryFilterData(qryFilterData);

  //Set the simulation filter data of the ground plane so that it collides with the chassis of a vehicle but not the wheels.
  PxFilterData simFilterData;
  simFilterData.word0 = COLLISION_FLAG_GROUND;
  simFilterData.word1 = COLLISION_FLAG_GROUND_AGAINST;
  shapes[0]->setSimulationFilterData(simFilterData);

  return groundPlane;
}




static PxConvexMesh* createConvexMesh(
    const PxVec3* verts,
    const PxU32 numVerts,
    PxPhysics& physics,
    PxCooking& cooking) {
    // Create descriptor for convex mesh
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count			= numVerts;
    convexDesc.points.stride		= sizeof(PxVec3);
    convexDesc.points.data			= verts;
    convexDesc.flags				= PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eINFLATE_CONVEX;

    PxConvexMesh* convexMesh = NULL;
    PxDefaultMemoryOutputStream buf;
    if(cooking.cookConvexMesh(convexDesc, buf))
    {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convexMesh = physics.createConvexMesh(id);
    }

    return convexMesh;
}

PxConvexMesh* createChassisMesh(
    const PxVec3 dims,
    PxPhysics& physics,
    PxCooking& cooking) {

  const PxF32 x = dims.x*0.5f;
  const PxF32 y = dims.y*0.5f;
  const PxF32 z = dims.z*0.5f;
  PxVec3 verts[8] =
  {
    PxVec3(x,y,-z),
    PxVec3(x,y,z),
    PxVec3(x,-y,z),
    PxVec3(x,-y,-z),
    PxVec3(-x,y,-z),
    PxVec3(-x,y,z),
    PxVec3(-x,-y,z),
    PxVec3(-x,-y,-z)
  };

  return createConvexMesh(verts,8,physics,cooking);

}

PxConvexMesh* createWheelMesh(
    const PxF32 width,
    const PxF32 radius,
    PxPhysics& physics,
    PxCooking& cooking) {

  PxVec3 points[2*16];
  for(PxU32 i = 0; i < 16; i++)
  {
      const PxF32 cosTheta = PxCos(i*PxPi*2.0f/16.0f);
      const PxF32 sinTheta = PxSin(i*PxPi*2.0f/16.0f);
      const PxF32 y = radius*cosTheta;
      const PxF32 z = radius*sinTheta;
      points[2*i+0] = PxVec3(-width/2.0f, y, z);
      points[2*i+1] = PxVec3(+width/2.0f, y, z);
  }

  return createConvexMesh(points,32,physics,cooking);
}

////////////////////////////////////////////////

PxRigidDynamic* createVehicleActor(
    const PxVehicleChassisData& chassisData,
    PxMaterial** wheelMaterials, PxConvexMesh** wheelConvexMeshes, const PxU32 numWheels,
    PxMaterial** chassisMaterials, PxConvexMesh** chassisConvexMeshes, const PxU32 numChassisMeshes,
    PxPhysics& physics) {

  //We need a rigid body actor for the vehicle.
  //Don't forget to add the actor to the scene after setting up the associated vehicle.
  PxRigidDynamic* vehActor = physics.createRigidDynamic(PxTransform(PxIdentity));

  //Wheel and chassis simulation filter data.
  PxFilterData wheelSimFilterData;
  wheelSimFilterData.word0 = COLLISION_FLAG_WHEEL;
  wheelSimFilterData.word1 = COLLISION_FLAG_WHEEL_AGAINST;
  PxFilterData chassisSimFilterData;
  chassisSimFilterData.word0 = COLLISION_FLAG_CHASSIS;
  chassisSimFilterData.word1 = COLLISION_FLAG_CHASSIS_AGAINST;

  //Wheel and chassis query filter data.
  //Optional: cars don't drive on other cars.
  PxFilterData wheelQryFilterData;
  setupNonDrivableSurface(wheelQryFilterData);
  PxFilterData chassisQryFilterData;
  setupNonDrivableSurface(chassisQryFilterData);

  //Add all the wheel shapes to the actor.
  for(PxU32 i = 0; i < numWheels; i++)
  {
    PxConvexMeshGeometry geom(wheelConvexMeshes[i]);
    PxShape* wheelShape=vehActor->createShape(geom, *wheelMaterials[i]);
    wheelShape->setQueryFilterData(wheelQryFilterData);
    wheelShape->setSimulationFilterData(wheelSimFilterData);
    wheelShape->setLocalPose(PxTransform(PxIdentity));
  }

  //Add the chassis shapes to the actor.
  for(PxU32 i = 0; i < numChassisMeshes; i++)
  {
    PxShape* chassisShape=vehActor->createShape
        (PxConvexMeshGeometry(chassisConvexMeshes[i]),*chassisMaterials[i]);
    chassisShape->setQueryFilterData(chassisQryFilterData);
    chassisShape->setSimulationFilterData(chassisSimFilterData);
    chassisShape->setLocalPose(PxTransform(PxIdentity));
  }

  vehActor->setMass(chassisData.mMass);
  vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
  vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset,PxQuat(PxIdentity)));

  return vehActor;
}



PxFilterFlags VehicleFilterShader(
    PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {

  PX_UNUSED(attributes0);
  PX_UNUSED(attributes1);
  PX_UNUSED(constantBlock);
  PX_UNUSED(constantBlockSize);

  if( (0 == (filterData0.word0 & filterData1.word1)) && (0 == (filterData1.word0 & filterData0.word1)) )
    return PxFilterFlag::eSUPPRESS;

  pairFlags = PxPairFlag::eCONTACT_DEFAULT;

  return PxFilterFlags();
}




void SetUpMatrix() {
  projMatrix	= kMath::Perspective(50.f, 1.0f * 640 / 480, 0.1f, 1000.0f);
  viewMatrix	= kMath::LookAt(kMath::Vec3f(2.0f, 2.f, 2.0f), kMath::Vec3f(0.0f, 0.0f, 0.0f), kMath::Vec3f(0.0f, 1.0f, 0.0f));
  modelMatrix = kMath::MakeIdentityMatrix<float>();
}

void LoadFX() {
  tech.ParseFXFromFile(PHONG_SHADER);
  g_prog_phong = tech.GetProgram("BlinnPhong");
  kDebug("g_prog_phong : %d\n", g_prog_phong);

  glUseProgram(g_prog_phong);
  gProj = glGetUniformLocation(g_prog_phong, "gProj");
  gWorld = glGetUniformLocation(g_prog_phong, "gWorld");
  gView = glGetUniformLocation(g_prog_phong, "gView");
  glUniformMatrix4fv(gWorld, 1, GL_FALSE, modelMatrix);
  glUseProgram(0);
}

void LoadMesh()
{
  k3dFile modelFile(TEST_MESH);
  if (!modelFile.Open(IORead)) {
    return;
  }

  Mesh::MeshHeader header;
  modelFile.Read((char*)&header, sizeof(header));
  kDebug("MeshVersion: %d\n", header.Version);

  k3dArchive arch;
  arch.SetIODevice(&modelFile);
  while (!modelFile.IsEOF()) {
    char name[64];
    modelFile.Read(name, 64);
    kDebug("Name: %s\n", name);
    if (strcmp(name, "k3dMesh")) break;
    k3dMesh *meshptr = new k3dMesh;
    arch >> (*meshptr);
    kDebug("Mesh Name %s\n", meshptr->MeshName());

    meshes[k3dString(meshptr->MeshName())] = (meshptr);
  }
}

void DumpMeshesInfo() {
  for(auto i : meshes) {
    kDebug("MeshName: %s->%d idc: %d\n", i.second->MeshName(), i.second->GetMaterialID(), i.second->GetIndexNum());
    kMath::AABB & bbox = i.second->GetBoundingBox();
    kDebug("MeshBound: Max(%.3f,%.3f,%.3f) Min(%.3f,%.3f,%.3f)\n",
           bbox.GetMaxCorner()[0],bbox.GetMaxCorner()[1],bbox.GetMaxCorner()[2],
        bbox.GetMinCorner()[0],bbox.GetMinCorner()[1],bbox.GetMinCorner()[2]);
  }
}

void InitVAOs() {
  for(auto i : meshes) {
    uint32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    CHECK_GL_ERROR
        GLuint vbo[4] = {0};
    glGenBuffers(4, vbo);
    CHECK_GL_ERROR

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
//    GLbitfield flags = GL_MAP_WRITE_BIT;
    glBufferData(GL_ARRAY_BUFFER, i.second->GetVertexNum()*sizeof(kMath::Vec3f), i.second->GetVertexBuffer(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    CHECK_GL_ERROR

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, i.second->GetVertexNum()*sizeof(kMath::Vec3f), i.second->GetNormalBuffer(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    CHECK_GL_ERROR

			glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, i.second->GetVertexNum()*sizeof(kMath::Vec2f), i.second->GetTexCoordBuffer(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(2);
		CHECK_GL_ERROR

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.second->GetIndexNum()*sizeof(uint32), i.second->GetIndexBuffer(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    vaos[i.first] = vao;
    CHECK_GL_ERROR
  }
}

static k3dMaterial *material = nullptr;

void LoadMaterial()
{
  material = new k3dMaterial;
  material->LoadMaterial(TEST_MATERIAL);
}

static void windowSize(GLFWwindow* window, int width, int height) {
  K3D_UNUSED(window);
  glViewport(0, 0, width, height);
  projMatrix = kMath::Perspective(50.f, 1.0f * width / height, 0.1f, 1000.0f);
  glProgramUniformMatrix4fv(g_prog_phong, gProj, 1, GL_FALSE, projMatrix);
}

bool leftpressed = false;
static double lastX = 0;
static double lastY = 0;
static bool firstPos = true;

static void buttonCallBack(GLFWwindow *window, int button, int action, int mods) {
  K3D_UNUSED(window);
  K3D_UNUSED(mods);
  if(button==GLFW_MOUSE_BUTTON_LEFT) {
    leftpressed = (action==GLFW_PRESS);
  }
}

static void cursurMove(GLFWwindow* window, double x, double y) {
  int w, h;
  glfwGetWindowSize(window, &w, &h);

  if (firstPos) {
    lastX = x; lastY = y;
    firstPos = false;
    return;
  }

  double deltaX = (x - lastX) / w;
  double deltaY = (y - lastY) / h;
  lastX = x; lastY = y;
  //kDebug("delta %.1f %.1f\n", deltaX, deltaY);
  static float theta = 0.f;
  static float phile = 0.f;

  if(leftpressed) {
    theta += (float)deltaX;
    phile += (float)deltaY;

    viewMatrix = kMath::LookAt(
          kMath::Vec3f( 3.0f*cos(theta)*cos(phile), 3.0f*sin(phile), 3.0f*sin(theta)*cos(phile) ),
          kMath::Vec3f(0.0f),
          kMath::Vec3f(0.0f, 1.0f, 0.0f)
          );
    if (g_prog_phong)
      glProgramUniformMatrix4fv(g_prog_phong, gView, 1, GL_FALSE, viewMatrix);
  }

}

int main(void)
{
  GLFWwindow* window;
  if (!glfwInit())
    return -1;
  window = glfwCreateWindow(640, 480, "MeshPrevewer", NULL, NULL);

  glfwSetWindowSizeCallback(window, windowSize);
  glfwSetCursorPosCallback(window, cursurMove);
  glfwSetMouseButtonCallback(window, buttonCallBack);

  if (!window)
  {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  k3dLog::InitLogFile("../../Data/MeshPreviewerLog.html");
  kPsEngine::Get().InitPhysics();

  gPhysics  = kPsEngine::Get().PhysicsPtr();
  gMaterial = kPsEngine::Get().DefaultMaterialPtr();
  gScene    = kPsEngine::Get().ScenePtr();


  gVehicleManager = new k3dVehicleManager(gScene);

  PxVehicleDrivableSurfaceType surfaceTypes[1];
  surfaceTypes[0].mType = SURFACE_TYPE_TARMAC;
  const PxMaterial* surfaceMaterials[1];
  surfaceMaterials[1] = kPsEngine::Get().DefaultMaterialPtr();
  gVehicleManager->Init(*(gPhysics), surfaceMaterials, surfaceTypes);


  gGroundPlane = createDrivablePlane(gMaterial, gPhysics);
  gScene->addActor(*gGroundPlane);

//  gScene->addActor(*gVehicle4W->getRigidDynamicActor());


  glewInit();

  SetUpMatrix();
  LoadFX();
  CHECK_GL_ERROR
  LoadMesh();
	DumpMeshesInfo();

	kVehicleDesc vehicleDesc;
	vehicleDesc.NumWheels = 4;
	vehicleDesc.WheelMass = 20;
	vehicleDesc.WheelRadius = 33;
	vehicleDesc.WheelWidth = 27;
	vehicleDesc.WheelMOI;
	vehicleDesc.ChassisMass = 1500;
	vehicleDesc.ChassisCMOffset;
	vehicleDesc.ChassisMOI;
	vehicleDesc.ChassisDims;

	kVehicle *vehicle = new kVehicle;
	kVehicle::SharedMeshPtr bodyShapeMesh(meshes[k3dString("bodyShape")]);
	vehicle->AddBodyMesh(bodyShapeMesh);
//    kVehicle::SharedMeshPtr badgeShapeMesh(meshes[k3dString("badgeShape⒇")]);
//	vehicle->AddBodyMesh(badgeShapeMesh);
    kVehicle::SharedMeshPtr coverShapeMesh(meshes[k3dString("coverShape")]);
	vehicle->AddBodyMesh(coverShapeMesh);
    kVehicle::SharedMeshPtr glassShapeMesh(meshes[k3dString("glassShape")]);
	vehicle->AddBodyMesh(glassShapeMesh);


	kVehicle::SharedMeshPtr w0(meshes[k3dString("LeftFrontWheel_Shape")]);
	kVehicle::SharedMeshPtr w1(meshes[k3dString("RightFrontWheel_Shape")]);
    kVehicle::SharedMeshPtr w2(meshes[k3dString("LeftRearWheel_Shape")]);
    kVehicle::SharedMeshPtr w3(meshes[k3dString("RightRearWheel_Shape")]);
	vehicle->AddWheelMesh(w0, 0);
	vehicle->AddWheelMesh(w1, 1);
	vehicle->AddWheelMesh(w2, 2);
	vehicle->AddWheelMesh(w3, 3);

	vehicle->CreateVehicle(vehicleDesc);

  InitVAOs();
  LoadMaterial();

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.4f, 0.4f, 0.8f, 1.0f);
  glViewport(0, 0, 640, 480);

  while (!glfwWindowShouldClose(window))
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* Render here */
    glUseProgram(g_prog_phong);

    for( auto i : meshes ) {
      material->ApplyMaterial(i.second->GetMaterialID(), projMatrix, viewMatrix, modelMatrix);
      glBindVertexArray(vaos[i.first]);
      glDrawElements(GL_TRIANGLES, i.second->GetIndexNum(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();



    const PxF32 timestep = 1.0f/60.0f;

    //Cycle through the driving modes to demonstrate how to accelerate/reverse/brake/turn etc.
//    incrementDrivingMode(timestep);

    {
//        PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(
//              gPadSmoothingData,
//              gSteerVsForwardSpeedTable,
//              gVehicleInputData,
//              timestep,
//              gIsVehicleInAir,
//              *gVehicle4W);
    }

    //Raycasts.
//    PxVehicleWheels* vehicles[1] = {gVehicle4W};
//    PxRaycastQueryResult* raycastResults = gVehicleSceneQueryData->getRaycastQueryResultBuffer(0);

//    const PxU32 raycastResultsSize = gVehicleSceneQueryData->getRaycastQueryResultBufferSize();

//    PxVehicleSuspensionRaycasts(gBatchQuery, 1, vehicles, raycastResultsSize, raycastResults);

//    //Vehicle update.
//    const PxVec3 grav = gScene->getGravity();

//    PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
//    PxVehicleWheelQueryResult vehicleQueryResults[1] = {
//      {wheelQueryResults, gVehicle4W->mWheelsSimData.getNbWheels()}
//    };

//    PxVehicleUpdates(timestep, grav, *gFrictionPairs, 1, vehicles, vehicleQueryResults);

//    //Work out if the vehicle is in the air.
//    gIsVehicleInAir = gVehicle4W->getRigidDynamicActor()->isSleeping() ? false : PxVehicleIsInAir(vehicleQueryResults[0]);
    //gVehicleManager->SuspensionRayCasts();
    gVehicleManager->Update(1.0f/60);

    kPsEngine::Get().UpdatePhysics(timestep);
  }

  glfwTerminate();

  gVehicleManager->Shutdown();

  kPsEngine::Get().ExitPhysics();
  k3dLog::CloseLog();
  return 0;
}
