#pragma once
#include <memory>
#include <vector>
#include <Physics/PxSupport.h>

struct TireFrictionMultipliers
{
  static float getValue(PxU32 surfaceType, PxU32 tireType)
  {
    //Tire model friction for each combination of drivable surface type and tire type.
    static PxF32 tireFrictionMultipliers[4][4]=
    {
      //WETS	SLICKS	ICE		MUD
      {0.95f,	0.95f,	0.95f,	0.95f},		//MUD
      {1.10f,	1.15f,	1.10f,	1.10f},		//TARMAC
      {0.70f,	0.70f,	0.70f,	0.70f},		//ICE
      {0.80f,	0.80f,	0.80f,	0.80f}		//GRASS
    };
    return tireFrictionMultipliers[surfaceType][tireType];
  }
};

class kVehicle;
///
/// \brief The k3dVehicleManager class
///
class k3dVehicleManager {
    typedef std::shared_ptr<kVehicle> SharedVehiclePtr;
public:

  k3dVehicleManager(PxScene *scene);
  ~k3dVehicleManager();

  ///
  /// \brief Init the vehicle manager
  /// \param physics
  /// \param drivableSurfaceMaterials
  /// \param drivableSurfaceTypes
  ///
  void Init(
      PxPhysics & physics,
      const PxMaterial** drivableSurfaceMaterials,
      const PxVehicleDrivableSurfaceType* drivableSurfaceTypes );

  ///
  /// \brief Shutdown
  ///
  void Shutdown();

  ///
  /// \brief AddVehicle
  /// \param vehicle
  ///
  void AddVehicle(SharedVehiclePtr &vehicle );

  ///
  /// \brief RemoveVehicle
  /// \param vehicle
  ///
  void RemoveVehicle(SharedVehiclePtr &vehicle );

  void SuspensionRayCasts();
  void Update( float deltaTime );
  void Update( float timeStep, const PxVec3 & gravity );
  void UpdateTuning();


  PxVehicleTelemetryData* GetTelemetryData();
  PxWheelQueryResult*     GetWheelsStates( SharedVehiclePtr & vehicle);

public:

  static void UpdateTireFrictionTable();

private:

  static bool                                                   m_bUpdateTireFrictionTable;
  static PxVehicleDrivableSurfaceToTireFrictionPairs*			m_SurfaceTirePairs;
  std::vector<SharedVehiclePtr>                                 m_kVehicles;
  std::vector<PxVehicleWheels*>									m_pVehicles;    // All instanced PhysX vehicles

  // Store each vehicle's wheels' states like isInAir, suspJounce, contactPoints, etc
  std::vector<PxVehicleWheelQueryResult>                        m_PVehiclesWheelsStates;
  // Scene query results for each wheel for each vehicle
  std::vector<PxRaycastQueryResult>                             m_WheelQueryResults;
  // Scene raycast hits for each wheel for each vehicle
  std::vector<PxRaycastHit>										m_WheelHitResults;
  PxBatchQuery*                                                 m_WheelRaycastBatchQuery;

  class SampleVehicleSceneQueryData*                            mSqData;

  PxSerializationRegistry*   m_SerializationRegistry;
  PxScene*                   m_Scene;

private:

  void UpdateTireFrictionTableInternal();
  void SetUpBatchedSceneQuery();
  void UpdateVehicles( float DeltaTime );
  PxVec3 GetSceneGravity();
};
