#include "kPsEngine.h"
#include <Core/k3dLog.h>
#include <Core/k3dDbg.h>


#define PVD_HOST "127.0.0.1"

#define KPX_SAFERELEASE(x) if(x){ x->release(); x = NULL;	}

kPsEngine::kPsEngine()
{
  m_IsInitialized = false;
  m_VehicleManager = nullptr;
}

kPsEngine::~kPsEngine()
{
  if(m_IsInitialized) {
    releaseInternalMembers();
  }
}

void kPsEngine::InitPhysics()
{
  k3dLog::Message("NVIDIA PhysX Version : %d.%d.%d. ", PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX );
  m_IsInitialized = initInternalMembers();
  //m_VehicleManager = new k3dVehicleManager(d->GetScene());
  //m_VehicleManager->Init(*GetPxSDK(), (const PxMaterial **)&d->i_Material, nullptr);
}

PxMaterial *kPsEngine::DefaultMaterialPtr()
{
  return m_Material;
}

PxFoundation* kPsEngine::FoundationPtr()
{
  return m_Foundation;
}

PxCooking *kPsEngine::Cooking()
{
  return m_Cooking;
}

void kPsEngine::AddActor(PxActor &actor)
{
  if(!m_IsInitialized) {
    kDebug("kPsEngine::d Error PIMP not initialized !!");
    return;
  }
  m_Scene->addActor(actor);
}

void kPsEngine::UpdatePhysics(float deltaTime)
{
  assert(m_IsInitialized);
  m_Scene->simulate(deltaTime);
  m_Scene->fetchResults(true);
}

void kPsEngine::ExitPhysics()
{
  if( m_IsInitialized ) {
    releaseInternalMembers();
  }
  k3dLog::Message("PhysX Engine Exiting...");
}

PxPhysics *kPsEngine::PhysicsPtr()
{
  return m_Physics;
}

PxScene *kPsEngine::ScenePtr()
{
  return m_Scene;
}


PxConvexMesh *kPsEngine::createConvexMesh(const PxVec3 *verts, const PxU32 numVerts)
{
  assert(sizeof(kMath::Vec3f) == sizeof(PxVec3));
  kPsEngine & psEngine = kPsEngine::Get();
  PxPhysics & physics = *( psEngine.m_Physics );
  PxCooking & cooking = *( psEngine.m_Cooking );

  PxConvexMeshDesc convexDesc;
  convexDesc.points.count         = numVerts;
  convexDesc.points.stride        = sizeof(PxVec3);
  convexDesc.points.data          = verts;

	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eINFLATE_CONVEX;

  PxConvexMesh* convexMesh = NULL;
  PxDefaultMemoryOutputStream buf;
  if(cooking.cookConvexMesh(convexDesc, buf))
  {
      PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
      convexMesh = physics.createConvexMesh(id);
  }
// save the convex mesh

  return convexMesh;
}

bool kPsEngine::initInternalMembers()
{
  PxTolerancesScale toleranceScale;
  m_Foundation  = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
  if(!m_Foundation) {
    k3dLog::Fatal("kPsEngine::Error PxCreateFoundation Failed!");
    return false;
  }

  PxProfileZoneManager* profileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_Foundation);
  m_Physics     = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, toleranceScale, true);

  PxCudaContextManagerDesc cudaContextManagerDesc;
  cudaContextManagerDesc.interopMode = PxCudaInteropMode::OGL_INTEROP;
//    cudaContextManagerDesc.graphicsDevice = getRenderer()->getDevice();
  m_CudaContextManager = PxCreateCudaContextManager(*m_Foundation, cudaContextManagerDesc, profileZoneManager);
  if( m_CudaContextManager )
  {
    if( !m_CudaContextManager->contextIsValid() )
    {
      m_CudaContextManager->release();
      m_CudaContextManager = nullptr;
    }
  }

  if(!PxInitExtensions(*m_Physics)) {
    k3dLog::Fatal("kPsEngine::Error PxInitExtensions Failed!");
    return false;
  }

  if(m_Physics->getPvdConnectionManager()) {
    m_Physics->getVisualDebugger()->setVisualizeConstraints(true);
    m_Physics->getVisualDebugger()->setVisualDebuggerFlags(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS | PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES);
    m_Connection = PxVisualDebuggerExt::createConnection(m_Physics->getPvdConnectionManager(), PVD_HOST, 5425, 10);
  }

  m_Connection = nullptr;

  PxCookingParams params(toleranceScale);
  params.meshWeldTolerance = 0.001f;
  params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES | PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
  m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, params);
  if(!m_Cooking) {
      k3dLog::Error("kPsEngine::Error PxCreateCooking failed!");
      return false;
  }

  m_Material = m_Physics->createMaterial(0.5f, 0.5f, 0.1f);

  PxSceneDesc sceneDesc( m_Physics->getTolerancesScale() );
  sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
  m_CPUDispatcher = PxDefaultCpuDispatcherCreate(4);

  sceneDesc.cpuDispatcher = m_CPUDispatcher;
  sceneDesc.filterShader = PxDefaultSimulationFilterShader;

  if(!sceneDesc.gpuDispatcher && m_CudaContextManager)
  {
    sceneDesc.gpuDispatcher = m_CudaContextManager->getGpuDispatcher();
  }
  m_Scene = m_Physics->createScene(sceneDesc);

  return true;
}

void kPsEngine::releaseInternalMembers()
{
  KPX_SAFERELEASE(m_Scene)
  KPX_SAFERELEASE(m_CPUDispatcher);
  KPX_SAFERELEASE(m_Cooking);
  PxCloseExtensions();
  PxProfileZoneManager* profileZoneManager = m_Physics->getProfileZoneManager();
  KPX_SAFERELEASE(m_Connection);
  KPX_SAFERELEASE(m_Physics);
  KPX_SAFERELEASE(m_CudaContextManager);
	if (profileZoneManager)
		profileZoneManager->release();
  m_Foundation->release();

  m_IsInitialized = false;
}
