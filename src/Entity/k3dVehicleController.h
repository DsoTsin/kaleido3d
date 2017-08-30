#pragma once
#include <Engine/k3dController.h>
#include "k3dVehicle.h"
#include <Physics/PxSupport.h>

///
/// \brief The kVehicleController class, inherit form controller, and composed with
/// a vehicle instance
///
class kVehicleController : public k3dController {
public:
  kVehicleController();
  virtual ~kVehicleController();

  void InitController();
  ///
  /// \brief HandleMessage is called in msg loop by MsgHandler
  /// \param msg
  ///
  void HandleMessage(SharedAppMsgHandlerPtr msg ) override;

  ///
  /// \brief Update is called in the engine loop.
  /// \param timestep
  /// \param vehicleWheelQueryResults
  /// \param focusVehicle
  ///
  void Update(const PxF32 timestep, const PxVehicleWheelQueryResult& vehicleWheelQueryResults, PxVehicleWheels& focusVehicle);

private:

  std::shared_ptr<kVehicle> m_VehiclePtr;

  bool  m_ToggleAutoGear;
  bool  m_Record;
  bool  m_Replay;
  bool  m_IsMovingForwardSlowly;
  bool  m_InReverseMode;

  bool  m_KeyPressedAccel;
  bool  m_KeyPressedBrake;
  bool  m_KeyPressedHandbrake;
  bool  m_KeyPressedSteerLeft;
  bool  m_KeyPressedSteerRight;
  bool  m_KeyPressedGearUp;
  bool  m_KeyPressedGearDown;

private:

  void processRawInputs(const PxF32 dtime, const bool useAutoGears, PxVehicleDrive4WRawInputData& rawInputData);

  void processAutoReverse(const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
      const PxVehicleDrive4WRawInputData& carRawInputs,
      bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const;
};
