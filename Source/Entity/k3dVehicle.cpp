#include "k3dVehicle.h"
#include <Physics/kPsEngine.h>

#include <Physics/k3dVehicleManager.h>
#include "Physics/kVehicleRaycast.h"

#include <Core/k3dDbg.h>

static PxVec3 computeChassisAABBDimensions(const PxConvexMesh* chassisConvexMesh)
{
  const PxU32 numChassisVerts = chassisConvexMesh->getNbVertices();
  const PxVec3* chassisVerts = chassisConvexMesh->getVertices();
  PxVec3 chassisMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
  PxVec3 chassisMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);
  for (PxU32 i = 0; i < numChassisVerts; i++)
  {
    chassisMin.x = PxMin(chassisMin.x, chassisVerts[i].x);
    chassisMin.y = PxMin(chassisMin.y, chassisVerts[i].y);
    chassisMin.z = PxMin(chassisMin.z, chassisVerts[i].z);
    chassisMax.x = PxMax(chassisMax.x, chassisVerts[i].x);
    chassisMax.y = PxMax(chassisMax.y, chassisVerts[i].y);
    chassisMax.z = PxMax(chassisMax.z, chassisVerts[i].z);
  }
  const PxVec3 chassisDims = chassisMax - chassisMin;
  return chassisDims;
}

static void computeWheelWidthsAndRadii(PxConvexMesh** wheelConvexMeshes, PxF32* wheelWidths, PxF32* wheelRadii)
{
  for (PxU32 i = 0; i < 4; i++)
  {
    const PxU32 numWheelVerts = wheelConvexMeshes[i]->getNbVertices();
    const PxVec3* wheelVerts = wheelConvexMeshes[i]->getVertices();
    PxVec3 wheelMin(PX_MAX_F32, PX_MAX_F32, PX_MAX_F32);
    PxVec3 wheelMax(-PX_MAX_F32, -PX_MAX_F32, -PX_MAX_F32);
    for (PxU32 j = 0; j < numWheelVerts; j++)
    {
      wheelMin.x = PxMin(wheelMin.x, wheelVerts[j].x);
      wheelMin.y = PxMin(wheelMin.y, wheelVerts[j].y);
      wheelMin.z = PxMin(wheelMin.z, wheelVerts[j].z);
      wheelMax.x = PxMax(wheelMax.x, wheelVerts[j].x);
      wheelMax.y = PxMax(wheelMax.y, wheelVerts[j].y);
      wheelMax.z = PxMax(wheelMax.z, wheelVerts[j].z);
    }
    wheelWidths[i] = wheelMax.x - wheelMin.x;
    wheelRadii[i] = PxMax(wheelMax.y, wheelMax.z)*0.975f;
  }
}

///
/// \brief createVehicle4WSimulationData
/// \param chassisMass
/// \param chassisConvexMesh
/// \param wheelMass
/// \param wheelConvexMeshes
/// \param wheelCentreOffsets
/// \param[in] wheelsData
/// \param[in] driveData
/// \param[in] chassisData
///
void createVehicle4WSimulationData(
    const PxF32 chassisMass, PxConvexMesh* chassisConvexMesh,
    const PxF32 wheelMass, PxConvexMesh** wheelConvexMeshes,
    const PxVec3* wheelCentreOffsets, PxVehicleWheelsSimData& wheelsData,
    PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData)
{
  //Extract the chassis AABB dimensions from the chassis convex mesh.
  const PxVec3 chassisDims = computeChassisAABBDimensions(chassisConvexMesh);

  //The origin is at the center of the chassis mesh.
  //Set the center of mass to be below this point and a little towards the front.
  const PxVec3 chassisCMOffset = PxVec3(0.0f, -chassisDims.y*0.5f + 0.65f, 0.25f);

  //Now compute the chassis mass and moment of inertia.
  //Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
  PxVec3 chassisMOI(
        (chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*chassisMass / 12.0f,
        (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*chassisMass / 12.0f,
        (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*chassisMass / 12.0f);
  //A bit of tweaking here.  The car will have more responsive turning if we reduce the
  //y-component of the chassis moment of inertia.
  chassisMOI.y *= 0.8f;

  //Let's set up the chassis data structure now.
  chassisData.mMass = chassisMass;
  chassisData.mMOI = chassisMOI;
  chassisData.mCMOffset = chassisCMOffset;

  //Compute the sprung masses of each suspension spring using a helper function.
  PxF32 suspSprungMasses[4];
  PxVehicleComputeSprungMasses(4, wheelCentreOffsets, chassisCMOffset, chassisMass, 1, suspSprungMasses);

  //Extract the wheel radius and width from the wheel convex meshes.
  PxF32 wheelWidths[4];
  PxF32 wheelRadii[4];
  computeWheelWidthsAndRadii(wheelConvexMeshes, wheelWidths, wheelRadii);

  kDebug("Wheel Width(%.3f) Radius(%.3f)\n", wheelWidths[0], wheelRadii[0]);

  //Now compute the wheel masses and inertias components around the axle's axis.
  //http://en.wikipedia.org/wiki/List_of_moments_of_inertia
  PxF32 wheelMOIs[4];
  for (PxU32 i = 0; i < 4; i++)
  {
    wheelMOIs[i] = 0.5f*wheelMass*wheelRadii[i] * wheelRadii[i];
  }
  //Let's set up the wheel data structures now with radius, mass, and moi.
  PxVehicleWheelData wheels[4];
  for (PxU32 i = 0; i < 4; i++)
  {
    wheels[i].mRadius = wheelRadii[i];
    wheels[i].mMass = wheelMass;
    wheels[i].mMOI = wheelMOIs[i];
    wheels[i].mWidth = wheelWidths[i];
  }
  //Disable the handbrake from the front wheels and enable for the rear wheels
  wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxHandBrakeTorque = 0.0f;
  wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxHandBrakeTorque = 0.0f;
  wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.0f;
  wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
  //Enable steering for the front wheels and disable for the front wheels.
  wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = PxPi*0.3333f;
  wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = PxPi*0.3333f;
  wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxSteer = 0.0f;
  wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxSteer = 0.0f;

  //Let's set up the tire data structures now.
  //Put slicks on the front tires and wets on the rear tires.
  PxVehicleTireData tires[4];
  tires[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mType = TIRE_TYPE_SLICKS;
  tires[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mType = TIRE_TYPE_SLICKS;
  tires[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mType = TIRE_TYPE_WETS;
  tires[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mType = TIRE_TYPE_WETS;

  //Let's set up the suspension data structures now.
  PxVehicleSuspensionData susps[4];
  for (PxU32 i = 0; i < 4; i++)
  {
    susps[i].mMaxCompression = 0.3f;
    susps[i].mMaxDroop = 0.1f;
    susps[i].mSpringStrength = 35000.0f;
    susps[i].mSpringDamperRate = 4500.0f;
  }
  susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_LEFT];
  susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT];
  susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_LEFT];
  susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mSprungMass = suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_RIGHT];

  //Set up the camber.
  //Remember that the left and right wheels need opposite camber so that the car preserves symmetry about the forward direction.
  //Set the camber to 0.0f when the spring is neither compressed or elongated.
  const PxF32 camberAngleAtRest = 0.0;
  susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtRest = camberAngleAtRest;
  susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtRest = -camberAngleAtRest;
  susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtRest = camberAngleAtRest;
  susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtRest = -camberAngleAtRest;
  //Set the wheels to camber inwards at maximum droop (the left and right wheels almost form a V shape)
  const PxF32 camberAngleAtMaxDroop = 0.001f;
  susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxDroop = camberAngleAtMaxDroop;
  susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
  susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxDroop = camberAngleAtMaxDroop;
  susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
  //Set the wheels to camber outwards at maximum compression (the left and right wheels almost form a A shape).
  const PxF32 camberAngleAtMaxCompression = -0.001f;
  susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxCompression = camberAngleAtMaxCompression;
  susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
  susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxCompression = camberAngleAtMaxCompression;
  susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxCompression = -camberAngleAtMaxCompression;

  //We need to set up geometry data for the suspension, wheels, and tires.
  //We already know the wheel centers described as offsets from the actor center and the center of mass offset from actor center.
  //From here we can approximate application points for the tire and suspension forces.
  //Lets assume that the suspension travel directions are absolutely vertical.
  //Also assume that we apply the tire and suspension forces 30cm below the center of mass.
  PxVec3 suspTravelDirections[4] = { PxVec3(0, -1, 0), PxVec3(0, -1, 0), PxVec3(0, -1, 0), PxVec3(0, -1, 0) };
  PxVec3 wheelCentreCMOffsets[4];
  PxVec3 suspForceAppCMOffsets[4];
  PxVec3 tireForceAppCMOffsets[4];
  for (PxU32 i = 0; i < 4; i++)
  {
    wheelCentreCMOffsets[i] = wheelCentreOffsets[i] - chassisCMOffset;
    suspForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
    tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
  }

  //Now add the wheel, tire and suspension data.
  for (PxU32 i = 0; i < 4; i++)
  {
    wheelsData.setWheelData(i, wheels[i]);
    wheelsData.setTireData(i, tires[i]);
    wheelsData.setSuspensionData(i, susps[i]);
    wheelsData.setSuspTravelDirection(i, suspTravelDirections[i]);
    wheelsData.setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
    wheelsData.setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
    wheelsData.setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
  }

  //Set the car to perform 3 sub-steps when it moves with a forwards speed of less than 5.0
  //and with a single step when it moves at speed greater than or equal to 5.0.
  wheelsData.setSubStepCount(5.0f, 3, 1);


  //Now set up the differential, engine, gears, clutch, and ackermann steering.

  //Diff
  PxVehicleDifferential4WData diff;
  diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
  driveData.setDiffData(diff);

  //Engine
  PxVehicleEngineData engine;
  engine.mPeakTorque = 500.0f;
  engine.mMaxOmega = 600.0f;//approx 6000 rpm
  driveData.setEngineData(engine);

  //Gears
  PxVehicleGearsData gears;
  gears.mSwitchTime = 0.5f;
  driveData.setGearsData(gears);

  //Clutch
  PxVehicleClutchData clutch;
  clutch.mStrength = 10.0f;
  driveData.setClutchData(clutch);

  //Ackermann steer accuracy
  PxVehicleAckermannGeometryData ackermann;
  ackermann.mAccuracy = 1.0f;
  ackermann.mAxleSeparation = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].z - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].z;
  ackermann.mFrontWidth = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x;
  ackermann.mRearWidth = wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].x;
  driveData.setAckermannGeometryData(ackermann);

}

kVehicle::kVehicle()
  : kSObject()
{
  SetNodeType(Node_Vehicle);
  m_WheelQueryResults = nullptr;
  m_VehicleQueryResults = nullptr;
  m_VehicleDrive = nullptr;
}

kVehicle::~kVehicle()
{

}

void kVehicle::AddBodyMesh(const kVehicle::SharedMeshPtr &bodyMeshPtr)
{
  PxConvexMesh * chassisMesh = kPsEngine::createConvexMesh( (PxVec3*)bodyMeshPtr->GetVertexBuffer(), bodyMeshPtr->GetVertexNum() );
  m_ChassisMesh.push_back(chassisMesh);
}

void kVehicle::AddWheelMesh(const kVehicle::SharedMeshPtr &wheelMeshPtr, int index)
{
  assert(index >= 0 && index < 4 && "Only for 4-wheels !");
  PxConvexMesh * wheelMesh = kPsEngine::createConvexMesh((PxVec3*)wheelMeshPtr->GetVertexBuffer(), wheelMeshPtr->GetVertexNum());
  kMath::AABB & aabbWheel = wheelMeshPtr->GetBoundingBox();
  const kMath::Vec3f & center = aabbWheel.GetCenter();
  PxVec3 wCenterOffset(center[0], center[1], center[2]);
  m_WheelCenterOffset[index] = wCenterOffset;
  m_WheelMesh[index] = wheelMesh;
}

void kVehicle::CreateVehicle(const kVehicleDesc &vehicleDesc)
{
  const PxVec3 chassisDims = computeChassisAABBDimensions(m_ChassisMesh[0]);
  const PxF32 wheelWidth = vehicleDesc.WheelWidth;
  const PxF32 wheelRadius = vehicleDesc.WheelRadius;
  const PxU32 numWheels = vehicleDesc.NumWheels;

  PxPhysics*        physics = kPsEngine::Get().PhysicsPtr();
  PxMaterial*       globalMaterial = kPsEngine::Get().DefaultMaterialPtr();
  //Construct a physx actor with shapes for the chassis and wheels.
  //Set the rigid body mass, moment of inertia, and center of mass offset.

  //Set up the sim data for the wheels.
  PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(numWheels);

  PxVehicleDriveSimData4W driveSimData;
  PxVehicleChassisData chassisData;
  createVehicle4WSimulationData(
        vehicleDesc.ChassisMass, m_ChassisMesh[0],
      20.0f, m_WheelMesh, m_WheelCenterOffset,
      *wheelsSimData, driveSimData, chassisData);


  //! Create Vehicle Actor
  PxRigidDynamic* veh4WActor = nullptr;
  {
    PxMaterial* wheelMaterials[PX_MAX_NB_WHEELS];
    for (PxU32 i = PxVehicleDrive4WWheelOrder::eFRONT_LEFT; i <= PxVehicleDrive4WWheelOrder::eREAR_RIGHT; i++)
    {
      wheelMaterials[i] = globalMaterial;
    }
    PxMaterial* chassisMaterials[5] = { globalMaterial, globalMaterial, globalMaterial, globalMaterial, globalMaterial };

    veh4WActor = createVehicleActor(
          chassisData,
          wheelMaterials, m_WheelMesh, numWheels,
          chassisMaterials, &m_ChassisMesh[0], m_ChassisMesh.size(),
        *physics);
  }

  //Create a vehicle from the wheels and drive sim data.
  PxVehicleDrive4W* vehDrive4W = PxVehicleDrive4W::allocate(numWheels);
  vehDrive4W->setup(physics, veh4WActor, *wheelsSimData, driveSimData, numWheels - 4);

  //Free the sim data because we don't need that any more.
  wheelsSimData->free();

  m_VehicleDrive = vehDrive4W;
}

PxVehicleWheelQueryResult *kVehicle::VehicleQueryResult()
{
  return m_VehicleQueryResults;
}

PxRigidDynamic *kVehicle::GetActor()
{
  return m_VehicleDrive->getRigidDynamicActor();
}

PxRigidDynamic* kVehicle::createVehicleActor(
    const PxVehicleChassisData &chassisData,
    PxMaterial **wheelMaterials,
    PxConvexMesh **wheelConvexMeshes,
    const PxU32 numWheels,
    PxMaterial **chassisMaterials,
    PxConvexMesh **chassisConvexMeshes,
    const PxU32 numChassisMeshes,
    PxPhysics &physics)
{

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
  for (PxU32 i = 0; i < numWheels; i++)
  {
    PxConvexMeshGeometry geom(wheelConvexMeshes[i]);
    PxShape* wheelShape = vehActor->createShape(geom, *wheelMaterials[i]);
    wheelShape->setQueryFilterData(wheelQryFilterData);
    wheelShape->setSimulationFilterData(wheelSimFilterData);
    wheelShape->setLocalPose(PxTransform(PxIdentity));
  }

  //Add the chassis shapes to the actor.
  for (PxU32 i = 0; i < numChassisMeshes; i++)
  {
    PxShape* chassisShape = vehActor->createShape(PxConvexMeshGeometry(chassisConvexMeshes[i]), *chassisMaterials[i]);

    chassisShape->setQueryFilterData(chassisQryFilterData);
    chassisShape->setSimulationFilterData(chassisSimFilterData);
    chassisShape->setLocalPose(PxTransform(PxIdentity));
  }

  vehActor->setMass(chassisData.mMass);
  vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
  vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset, PxQuat(PxIdentity)));

  return vehActor;
}
