#pragma once
#include <KTL/Singleton.h>
#include <Physics/k3dVehicleManager.h>

typedef std::shared_ptr<PxPhysics> PxSDKPtr;

///
/// \brief The kPsEngine class
/// an aggregator of physx engine
///
class kPsEngine : public Singleton<kPsEngine> {
public:
  kPsEngine();
  ~kPsEngine();

  void      InitPhysics();
  void      ExitPhysics();

  ///
  /// \brief Get PxPhysics Ptr
  /// \return PxPhysics's pointer
  ///
  PxPhysics*  PhysicsPtr();

  ///
  /// \brief Get PxScene Ptr
  /// \return PxScene's pointer
  ///
  PxScene*    ScenePtr();

  ///
  /// \brief Get default material
  /// \return PxMaterial*
  ///
  PxMaterial * DefaultMaterialPtr();

  ///
  /// \brief Get Foundation Ptr
  /// \return
  ///
  PxFoundation *FoundationPtr();


  PxCooking * Cooking();
  ///
  /// \brief AddActor
  /// \param actor PxActor &
  ///
  void      AddActor(PxActor & actor);

  ///
  /// \brief UpdatePhysics
  /// \param deltaTime
  ///
  void      UpdatePhysics(float deltaTime);


public:

  ///
  /// \brief cook raw mesh into physics asset
  /// \param verts
  /// \param numVerts
  /// \return PxConvexMesh*
  ///
  static PxConvexMesh* createConvexMesh(
      const PxVec3* verts, const PxU32 numVerts);

private:
  bool                    initInternalMembers();
  void                    releaseInternalMembers();

  k3dVehicleManager*      m_VehicleManager;

  PxDefaultAllocator      m_Allocator;
  PxDefaultErrorCallback  m_ErrorCallback;
  PxFoundation*           m_Foundation;
  PxPhysics*              m_Physics;
  PxDefaultCpuDispatcher* m_CPUDispatcher;
  PxCudaContextManager*   m_CudaContextManager;
  PxScene*                m_Scene;
  PxVisualDebuggerConnection * m_Connection;
  PxCooking*              m_Cooking;
  PxMaterial*             m_Material;

  mutable bool            m_IsInitialized;
};
