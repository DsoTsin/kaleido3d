#include "k3dVehicleController.h"

#ifdef USE_GLFW
#include <Platform/GLFW/kFMsgHandler.h>
#endif
// k3dVehicleController

PxVehicleKeySmoothingData gKeySmoothingData=
{
  {
    3.0f,	//rise rate eANALOG_INPUT_ACCEL
    3.0f,	//rise rate eANALOG_INPUT_BRAKE
    10.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
    2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
    2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
  },
  {
    5.0f,	//fall rate eANALOG_INPUT__ACCEL
    5.0f,	//fall rate eANALOG_INPUT__BRAKE
    10.0f,	//fall rate eANALOG_INPUT__HANDBRAKE
    5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
    5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
  }
};

PxVehiclePadSmoothingData gCarPadSmoothingData=
{
  {
    6.0f,	//rise rate eANALOG_INPUT_ACCEL
    6.0f,	//rise rate eANALOG_INPUT_BRAKE
    12.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
    2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
    2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
  },
  {
    10.0f,	//fall rate eANALOG_INPUT_ACCEL
    10.0f,	//fall rate eANALOG_INPUT_BRAKE
    12.0f,	//fall rate eANALOG_INPUT_HANDBRAKE
    5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
    5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
  }
};

PxF32 gSteerVsForwardSpeedData[2*8]=
{
  0.0f,		0.75f,
  5.0f,		0.75f,
  30.0f,		0.125f,
  120.0f,		0.1f,
  PX_MAX_F32, PX_MAX_F32,
  PX_MAX_F32, PX_MAX_F32,
  PX_MAX_F32, PX_MAX_F32,
  PX_MAX_F32, PX_MAX_F32
};
PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData,4);

//Tank smoothing data.
PxVehiclePadSmoothingData gTankPadSmoothingData=
{
  {
    6.0f,		//rise rate eTANK_ANALOG_INPUT_ACCEL
    6.0f,		//rise rate eTANK_ANALOG_INPUT_BRAKE_LEFT
    6.0f,		//rise rate eTANK_ANALOG_INPUT_BRAKE_RIGHT
    2.5f,		//rise rate eTANK_ANALOG_INPUT_THRUST_LEFT
    2.5f,		//rise rate eTANK_ANALOG_INPUT_THRUST_RIGHT
  },
  {
    10.0f,		//fall rate eTANK_ANALOG_INPUT_ACCEL
    10.0f,		//fall rate eTANK_ANALOG_INPUT_BRAKE_LEFT
    10.0f,		//fall rate eTANK_ANALOG_INPUT_BRAKE_RIGHT
    5.0f,		//fall rate eTANK_ANALOG_INPUT_THRUST_LEFT
    5.0f		//fall rate eTANK_ANALOG_INPUT_THRUST_RIGHT
  }
};


kVehicleController::kVehicleController()
{
  m_ToggleAutoGear  = false;
  m_Record          = false;
  m_Replay          = false;
  m_IsMovingForwardSlowly = true;
  m_InReverseMode   = false;

  m_KeyPressedAccel       = false;
  m_KeyPressedBrake       = false;
  m_KeyPressedHandbrake   = false;
  m_KeyPressedSteerLeft   = false;
  m_KeyPressedSteerRight  = false;
  m_KeyPressedGearUp      = false;
  m_KeyPressedGearDown    = false;
}

kVehicleController::~kVehicleController()
{
}

void kVehicleController::InitController()
{

}

void kVehicleController::HandleMessage(SharedAppMsgHandlerPtr  msg)
{
#ifdef USE_GLFW
  std::shared_ptr<kFMsgHandler> & fMsg = std::static_pointer_cast<kFMsgHandler>(msg);
  uint32 key = fMsg->GetKeyState();

  m_KeyPressedAccel       = key&EKeyState::Key_WPressed;
  m_KeyPressedBrake       = key&EKeyState::Key_SPressed;
  m_KeyPressedHandbrake   = key&EKeyState::Key_ZPressed;
  m_KeyPressedSteerLeft   = key&EKeyState::Key_APressed;
  m_KeyPressedSteerRight  = key&EKeyState::Key_DPressed;
  m_KeyPressedGearUp      = key&EKeyState::Key_QPressed;
  m_KeyPressedGearDown    = key&EKeyState::Key_EPressed;
#endif
}

void kVehicleController::Update(const PxF32 timestep, const PxVehicleWheelQueryResult &vehicleWheelQueryResults, PxVehicleWheels &focusVehicle)
{
  PxVehicleDriveDynData* driveDynData = nullptr;
  bool isTank = false;
  PxVehicleDriveTankControlModel::Enum tankDriveModel = PxVehicleDriveTankControlModel::eSTANDARD;
  switch(focusVehicle.getVehicleType())
  {
  case PxVehicleTypes::eDRIVE4W:
  {
    PxVehicleDrive4W& vehDrive4W = (PxVehicleDrive4W&)focusVehicle;
    driveDynData = &vehDrive4W.mDriveDynData;
    isTank=false;
  }
    break;
  case PxVehicleTypes::eDRIVENW:
  {
    PxVehicleDriveNW& vehDriveNW=(PxVehicleDriveNW&)focusVehicle;
    driveDynData=&vehDriveNW.mDriveDynData;
    isTank=false;
  }
    break;
  case PxVehicleTypes::eDRIVETANK:
  {
    PxVehicleDriveTank& vehDriveTank=(PxVehicleDriveTank&)focusVehicle;
    driveDynData=&vehDriveTank.mDriveDynData;
    isTank=true;
    tankDriveModel=vehDriveTank.getDriveModel();
  }
    break;
  default:
    assert(false);
    break;
  }

  if(m_ToggleAutoGear)
  {
    driveDynData->toggleAutoGears();
    m_ToggleAutoGear = false;
  }

  //Store raw inputs in replay stream if in recording mode.
  //Set raw inputs from replay stream if in replay mode.
  //Store raw inputs from active stream in handy arrays so we don't need to worry
  //about which stream (live input or replay) is active.
  //Work out if we are using keys or gamepad controls depending on which is being used
  //(gamepad selected if both are being used).
  PxVehicleDrive4WRawInputData carRawInputs;
  if(!isTank)
  {
    processRawInputs(timestep, driveDynData->getUseAutoGears(), carRawInputs);
  }

  //Work out if the car is to flip from reverse to forward gear or from forward gear to reverse.
  bool toggleAutoReverse = false;
  bool newIsMovingForwardSlowly = false;
  if(!isTank)
  {
    processAutoReverse(focusVehicle, *driveDynData, vehicleWheelQueryResults, carRawInputs, toggleAutoReverse, newIsMovingForwardSlowly);
  }

  m_IsMovingForwardSlowly = newIsMovingForwardSlowly;


  //If the car is to flip gear direction then switch gear as appropriate.
  if(toggleAutoReverse)
  {
    m_InReverseMode = !m_InReverseMode;

    if(m_InReverseMode)
    {
      driveDynData->forceGearChange(PxVehicleGearsData::eREVERSE);
    }
    else
    {
      driveDynData->forceGearChange(PxVehicleGearsData::eFIRST);
    }
  }

  //If in reverse mode then swap the accel and brake.
  if(m_InReverseMode)
  {
    if(!isTank)
    {
      const bool accel = carRawInputs.getDigitalAccel();
      const bool brake = carRawInputs.getDigitalBrake();
      carRawInputs.setDigitalAccel(brake);
      carRawInputs.setDigitalBrake(accel);
    }
  }

  // Now filter the raw input values and apply them to focus vehicle
  // as floats for brake,accel,handbrake,steer and bools for gearup,geardown.
  if(!isTank)
  {
    const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
    PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs(
          gKeySmoothingData,
          gSteerVsForwardSpeedTable,
          carRawInputs,
          timestep,
          isInAir,
          (PxVehicleDrive4W&)focusVehicle);
  }

}

void kVehicleController::processRawInputs(
    const PxF32 dtime,
    const bool useAutoGears,
    PxVehicleDrive4WRawInputData &rawInputData)
{
  K3D_UNUSED(dtime);
  // Keyboard
  {
    rawInputData.setDigitalAccel(m_KeyPressedAccel);  // Accel -> KeyA
    rawInputData.setDigitalBrake(m_KeyPressedBrake);  // Brake -> KeyZ
    rawInputData.setDigitalHandbrake(m_KeyPressedHandbrake); // HandBrake -> KeySpace
    rawInputData.setDigitalSteerLeft(m_KeyPressedSteerLeft); // SteerLeft -> ArrowLeft
    rawInputData.setDigitalSteerRight(m_KeyPressedSteerRight); // SteerRight -> ArrowRight
    rawInputData.setGearUp(m_KeyPressedGearUp);  // GearUp -> ArrowUp
    rawInputData.setGearDown(m_KeyPressedGearDown); // GearDown -> ArrowDown

//    mUseKeyInputs=
//        (mKeyPressedAccel || mKeyPressedBrake  || mKeyPressedHandbrake ||
//         mKeyPressedSteerLeft || mKeyPressedSteerRight ||
//         mKeyPressedGearUp || mKeyPressedGearDown);
  }

  if(useAutoGears && (rawInputData.getGearDown() || rawInputData.getGearUp()))
  {
    rawInputData.setGearDown(false);
    rawInputData.setGearUp(false);
  }

}

#define THRESHOLD_FORWARD_SPEED (0.1f)
#define THRESHOLD_SIDEWAYS_SPEED (0.2f)
#define THRESHOLD_ROLLING_BACKWARDS_SPEED (0.1f)

void kVehicleController::processAutoReverse(
    const PxVehicleWheels &focusVehicle,
    const PxVehicleDriveDynData &driveDynData,
    const PxVehicleWheelQueryResult &vehicleWheelQueryResults,
    const PxVehicleDrive4WRawInputData &carRawInputs,
    bool &toggleAutoReverse,
    bool &newIsMovingForwardSlowly) const
{
  newIsMovingForwardSlowly = false;
  toggleAutoReverse = false;

  if(driveDynData.getUseAutoGears())
  {
    //If the car is travelling very slowly in forward gear without player input and the player subsequently presses the brake then we want the car to go into reverse gear
    //If the car is travelling very slowly in reverse gear without player input and the player subsequently presses the accel then we want the car to go into forward gear
    //If the car is in forward gear and is travelling backwards then we want to automatically put the car into reverse gear.
    //If the car is in reverse gear and is travelling forwards then we want to automatically put the car into forward gear.
    //(If the player brings the car to rest with the brake the player needs to release the brake then reapply it
    //to indicate they want to toggle between forward and reverse.)

    const bool prevIsMovingForwardSlowly  = m_IsMovingForwardSlowly;
    bool isMovingForwardSlowly            = false;
    bool isMovingBackwards                = false;
    const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
    if(!isInAir)
    {
      bool accelRaw, brakeRaw, handbrakeRaw;

      accelRaw      = carRawInputs.getDigitalAccel();
      brakeRaw      = carRawInputs.getDigitalBrake();
      handbrakeRaw  = carRawInputs.getDigitalHandbrake();

      const PxF32 forwardSpeed      = focusVehicle.computeForwardSpeed();
      const PxF32 forwardSpeedAbs   = PxAbs(forwardSpeed);
      const PxF32 sidewaysSpeedAbs  = PxAbs(focusVehicle.computeSidewaysSpeed());
      const PxU32 currentGear       = driveDynData.getCurrentGear();
      const PxU32 targetGear        = driveDynData.getTargetGear();

      //Check if the car is rolling against the gear (backwards in forward gear or forwards in reverse gear).
      if(PxVehicleGearsData::eFIRST == currentGear  && forwardSpeed < -THRESHOLD_ROLLING_BACKWARDS_SPEED)
      {
        isMovingBackwards = true;
      }
      else if(PxVehicleGearsData::eREVERSE == currentGear && forwardSpeed > THRESHOLD_ROLLING_BACKWARDS_SPEED)
      {
        isMovingBackwards = true;
      }

      //Check if the car is moving slowly.
      if(forwardSpeedAbs < THRESHOLD_FORWARD_SPEED && sidewaysSpeedAbs < THRESHOLD_SIDEWAYS_SPEED)
      {
        isMovingForwardSlowly=true;
      }

      //Now work if we need to toggle from forwards gear to reverse gear or vice versa.
      if(isMovingBackwards)
      {
        if(!accelRaw && !brakeRaw && !handbrakeRaw && (currentGear == targetGear))
        {
          //The car is rolling against the gear and the player is doing nothing to stop this.
          toggleAutoReverse = true;
        }
      }
      else if(prevIsMovingForwardSlowly && isMovingForwardSlowly)
      {
        if((currentGear > PxVehicleGearsData::eNEUTRAL) && brakeRaw && !accelRaw && (currentGear == targetGear))
        {
          //The car was moving slowly in forward gear without player input and is now moving slowly with player input that indicates the
          //player wants to switch to reverse gear.
          toggleAutoReverse = true;
        }
        else if(currentGear == PxVehicleGearsData::eREVERSE && accelRaw && !brakeRaw && (currentGear == targetGear))
        {
          //The car was moving slowly in reverse gear without player input and is now moving slowly with player input that indicates the
          //player wants to switch to forward gear.
          toggleAutoReverse = true;
        }
      }

      //If the car was brought to rest through braking then the player needs to release the brake then reapply
      //to indicate that the gears should toggle between reverse and forward.
      if(isMovingForwardSlowly && !brakeRaw && !accelRaw && !handbrakeRaw)
      {
        newIsMovingForwardSlowly = true;
      }
    }
  }
}
