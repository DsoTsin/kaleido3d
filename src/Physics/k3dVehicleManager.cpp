#include "k3dVehicleManager.h"
#include "Entity/k3dVehicle.h"
#include "kVehicleSceneQuery.h"
#include <algorithm>

PxVehicleDrivableSurfaceToTireFrictionPairs*
k3dVehicleManager::m_SurfaceTirePairs = nullptr;

const int MAX_NUM_PENDULA = 4;

PxRevoluteJoint* gRevoluteJoints[MAX_NUM_PENDULA]={NULL,NULL,NULL,NULL};
PxF32 gRevoluteJointDriveSpeeds[MAX_NUM_PENDULA]={0.932f,1.0f,1.237f,0.876f};
PxF32 gRevoluteJointTimers[MAX_NUM_PENDULA]={0,0,0,0};
PxU32 gNumRevoluteJoints=0;
PxF32 gRevoluteJointMaxTheta=0;

k3dVehicleManager::k3dVehicleManager(PxScene *scene)
{
  m_SerializationRegistry = nullptr;
  m_Scene                 = scene;
}

k3dVehicleManager::~k3dVehicleManager()
{

}

void k3dVehicleManager::Init(
    PxPhysics &physics,
    const PxMaterial** drivableSurfaceMaterials,
    const PxVehicleDrivableSurfaceType* drivableSurfaceTypes )
{
  m_SerializationRegistry = PxSerialization::createSerializationRegistry(physics);
  PxInitVehicleSDK(physics, m_SerializationRegistry);
  PxVec3 up(0,1,0);
  PxVec3 forward(0,0,1);
  PxVehicleSetBasisVectors(up,forward);

  //Set the vehicle update mode to be immediate velocity changes.
  PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

  m_SurfaceTirePairs = PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(4, 4);
  m_SurfaceTirePairs->setup(4, 4, drivableSurfaceMaterials, drivableSurfaceTypes);
  for(PxU32 i=0; i<4; i++)
  {
    for(PxU32 j=0; j<4; j++)
    {
      m_SurfaceTirePairs->setTypePairFriction(i,j,TireFrictionMultipliers::getValue(i, j));
    }
  }
}

void k3dVehicleManager::Shutdown()
{
  PxCloseVehicleSDK(m_SerializationRegistry);

  if(m_SerializationRegistry)
    m_SerializationRegistry->release();
}

void k3dVehicleManager::AddVehicle(k3dVehicleManager::SharedVehiclePtr &vehicle)
{
  m_kVehicles.push_back(vehicle);
}

void k3dVehicleManager::RemoveVehicle(k3dVehicleManager::SharedVehiclePtr &vehicle)
{
  m_kVehicles.erase(std::find(m_kVehicles.begin(), m_kVehicles.end(), vehicle));
}

void k3dVehicleManager::SuspensionRayCasts()
{
  //Create a scene query if we haven't already done so.
  if(nullptr == m_WheelRaycastBatchQuery)
  {
    m_WheelRaycastBatchQuery = mSqData->setUpBatchedSceneQuery(m_Scene);
  }
  //Raycasts.
  PxSceneReadLock scopedLock(*m_Scene);
  PxVehicleSuspensionRaycasts(
        m_WheelRaycastBatchQuery,
        m_pVehicles.size(), &m_pVehicles[0],
        mSqData->getRaycastQueryResultBufferSize(),
        mSqData->getRaycastQueryResultBuffer());

}

void k3dVehicleManager::UpdateTireFrictionTable()
{

}

void k3dVehicleManager::Update(float deltaTime)
{
  //Update the vehicle controls. called in engine loop
//  mVehicleController.setCarKeyboardInputs(
//        mControlInputs.getAccelKeyPressed(),
//        mControlInputs.getBrakeKeyPressed(),
//        mControlInputs.getHandbrakeKeyPressed(),
//        mControlInputs.getSteerLeftKeyPressed(),
//        mControlInputs.getSteerRightKeyPressed(),
//        mControlInputs.getGearUpKeyPressed(),
//        mControlInputs.getGearDownKeyPressed());
//  mVehicleController.setCarGamepadInputs(
//        mControlInputs.getAccel(),
//        mControlInputs.getBrake(),
//        mControlInputs.getSteer(),
//        mControlInputs.getGearUp(),
//        mControlInputs.getGearDown(),
//        mControlInputs.getHandbrake());

////    updateVehicleController(deltaTime);

//    //Update the vehicles.
//    mVehicleManager.suspensionRaycasts(&getActiveScene());

//    if (deltaTime > 0.0f)
//    {
//      PxSceneWriteLock scopedLock( *m_Scene );
//#if PX_DEBUG_VEHICLE_ON
//      updateVehicleManager(deltaTime, getActiveScene().getGravity());
//#else
//      mVehicleManager.update(deltaTime,getActiveScene().getGravity());
//#endif
//    }

    //Update the camera.
//    mCameraController.setInputs(
//          mControlInputs.getRotateY(),
//          mControlInputs.getRotateZ());
//    updateCameraController(deltaTime,getActiveScene());

    //Update the revolute joints.
    //If the joint has exceeded the rotation limit then reverse the drive velocity
    //to make the joint rotate in the opposite direction.
    for(PxU32 i=0;i<gNumRevoluteJoints;i++)
    {
      PxSceneWriteLock scopedLock( *m_Scene );
      //Get the two actors of the joint.
      PxRigidActor* actor0=NULL;
      PxRigidActor* actor1=NULL;
      gRevoluteJoints[i]->getActors(actor0,actor1);

      //Work out the rotation angle of the joint.
      const PxF32 cosTheta = PxAbs(actor1->is<PxRigidDynamic>()->getGlobalPose().q.getBasisVector1().y);
      const PxF32 theta = PxAcos(cosTheta);

      //If the joint rotation limit has been exceeded then reverse the drive direction.
      //It's possible to reverse the direction but then after a timestep or two for it to still be beyond the rotation limit.
      //To avoid switching back and forth don't switch drive direction until a minimum time has passed since the last drive direction change.
      //Its possible that the pendulum has hit the car and is unable to reach the limit.  A nice fix for this is to keep a track of the time
      //passed since the last direction change and reverse the joint drive direction if a time limit has been reached.  This gives the car
      //a chance to escape the pendulum.
      if((theta > gRevoluteJointMaxTheta && gRevoluteJointTimers[i]>4*deltaTime) || gRevoluteJointTimers[i] > 4*gRevoluteJointMaxTheta/gRevoluteJointDriveSpeeds[i])
      {
        //Help the joint by setting the actor momenta to zero.
        ((PxRigidDynamic*)actor1)->setLinearVelocity(PxVec3(0,0,0));
        ((PxRigidDynamic*)actor1)->setAngularVelocity(PxVec3(0,0,0));

        //Switch the joint drive direction.
        const PxF32 currDriveVel=gRevoluteJoints[i]->getDriveVelocity();
        const PxF32 newDriveVel=-currDriveVel;
        gRevoluteJoints[i]->setDriveVelocity(newDriveVel);

        //Reset the timer.
        gRevoluteJointTimers[i]=0;
      }

      //Increment the joint timer.
      gRevoluteJointTimers[i] += deltaTime;
    }

    {
      PxSceneReadLock scopedLock(*m_Scene);

      //Update the progress around the track with the latest vehicle transform.
//      PxRigidDynamic* actor = getFocusVehicleRigidDynamicActor();
//      mWayPoints.update(actor->getGlobalPose(), deltaTime);

      //Cache forward speed for the HUD to avoid making API calls while vehicle update is running
//      const PxVehicleWheels& focusVehicle = *mVehicleManager.getVehicle(mPlayerVehicle);
//      mForwardSpeedHud = focusVehicle.computeForwardSpeed();
    }
}

void k3dVehicleManager::Update(float timeStep, const PxVec3 &gravity)
{
  PxVehicleUpdates( timeStep, gravity, *m_SurfaceTirePairs,
                    m_pVehicles.size(), &m_pVehicles[0], &m_PVehiclesWheelsStates[0] );
}

PxVehicleTelemetryData *k3dVehicleManager::GetTelemetryData()
{
  return nullptr;
}

PxWheelQueryResult *k3dVehicleManager::GetWheelsStates(k3dVehicleManager::SharedVehiclePtr &vehicle)
{
  return nullptr;
}

