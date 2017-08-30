#pragma once
#include <Math/kMath.hpp>
#include <Physics/PxSupport.h>
#include <Core/k3dMesh.h>
#include <Engine/k3dSceneObject.h>

//Tire types.
//enum
//{
//    TIRE_TYPE_NORMAL=0,
//    TIRE_TYPE_WORN,
//    MAX_NUM_TIRE_TYPES
//};
//Tire types.
enum
{
    TIRE_TYPE_WETS=0,
    TIRE_TYPE_SLICKS,
    TIRE_TYPE_ICE,
    TIRE_TYPE_MUD,
    MAX_NUM_TIRE_TYPES
};

enum
{
  COLLISION_FLAG_GROUND			=	1 << 0,
  COLLISION_FLAG_WHEEL			=	1 << 1,
  COLLISION_FLAG_CHASSIS			=	1 << 2,
  COLLISION_FLAG_OBSTACLE			=	1 << 3,
  COLLISION_FLAG_DRIVABLE_OBSTACLE=	1 << 4,

  COLLISION_FLAG_GROUND_AGAINST	=															COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
  COLLISION_FLAG_WHEEL_AGAINST	=									COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
  COLLISION_FLAG_CHASSIS_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
  COLLISION_FLAG_OBSTACLE_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
  COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST=	COLLISION_FLAG_GROUND 						 |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};

///
/// \brief The kVehicleDesc struct has
/// chassisMass, chassisDims, chassisMOI, chassCMOffset
/// NumWheels, WheelWidth, WheelMass, WheelRadius, WheelMOI
struct kVehicleDesc {
  float         ChassisMass;
  PxVec3  ChassisDims;
  PxVec3  ChassisMOI;
  PxVec3  ChassisCMOffset;

  int   NumWheels; // default set to 4 wheels
  float WheelMass;
  float WheelWidth;
  float WheelRadius;
  float WheelMOI;
};

class kVehicle  : public kSObject {
public:
  kVehicle();
  ~kVehicle();

  typedef std::shared_ptr<k3dMesh> SharedMeshPtr;
  ///
  /// \brief Add the body mesh of vehicle
  /// \param bodyMeshPtr
  ///
  void AddBodyMesh( const SharedMeshPtr & bodyMeshPtr );

  ///
  /// \brief Add wheel mesh for ech wheel
  /// \param[in] wheelMeshPtr
  /// \param index
  ///
  void AddWheelMesh( const SharedMeshPtr & wheelMeshPtr, int index);

  ///
  /// \brief create vehicle instance with mesh
  /// \param vehicleDesc
  /// \param mesh
  ///
  void CreateVehicle( const kVehicleDesc & vehicleDesc );

  ///
  /// \brief WheelQueryResult
  /// \return
  ///
  PxVehicleWheelQueryResult*  VehicleQueryResult();

  ///
  /// \brief GetActor
  /// \return
  ///
  PxRigidDynamic  * GetActor();

private:

  kVehicleDesc                m_VehicleDesc;
  PxWheelQueryResult *        m_WheelQueryResults; // elements num same as num wheels
  PxVehicleWheelQueryResult * m_VehicleQueryResults/* = {{wheelQueryResults, gVehicle4W->mWheelsSimData.getNbWheels()}}*/;
  PxVehicleDrive *            m_VehicleDrive;
  PxConvexMesh*               m_WheelMesh[4];
  PxVec3                      m_WheelCenterOffset[4];
  std::vector<PxConvexMesh*>  m_ChassisMesh;

private:

  PxRigidDynamic *createVehicleActor(
      const PxVehicleChassisData& chassisData,
       PxMaterial** wheelMaterials, PxConvexMesh** wheelConvexMeshes, const PxU32 numWheels,
       PxMaterial** chassisMaterials, PxConvexMesh** chassisConvexMeshes, const PxU32 numChassisMeshes,
       PxPhysics& physics);
};
