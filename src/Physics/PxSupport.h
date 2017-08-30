#ifndef __PxSupport_h__
#define __PxSupport_h__
#pragma once

#include "PxPhysicsAPI.h"
#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleDrive4W.h"
#include "vehicle/PxVehicleDriveNW.h"
#include "vehicle/PxVehicleDriveTank.h"
#include "vehicle/PxVehicleUpdate.h"
#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleUtilSetup.h"
#include "vehicle/PxVehicleUtil.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "geometry/PxConvexMesh.h"
#include "geometry/PxConvexMeshGeometry.h"
#include "PxSerialization.h"
#include "PxVehicleSerialization.h"
#include <Math/kMath.hpp>

using namespace physx;

inline void ConvertPxMat44toMat4f( physx::PxMat44 & pmat, kMath::Mat4f & kmat )
{
  kmat[0][0] = pmat.column0[0]; kmat[0][1] = pmat.column0[1]; kmat[0][2] = pmat.column0[2]; kmat[0][3] = pmat.column0[3];
  kmat[1][0] = pmat.column1[0]; kmat[1][1] = pmat.column1[1]; kmat[1][2] = pmat.column1[2]; kmat[1][3] = pmat.column1[3];
  kmat[2][0] = pmat.column2[0]; kmat[2][1] = pmat.column2[1]; kmat[2][2] = pmat.column2[2]; kmat[2][3] = pmat.column2[3];
  kmat[3][0] = pmat.column3[0]; kmat[3][1] = pmat.column3[1]; kmat[3][2] = pmat.column3[2]; kmat[3][3] = pmat.column3[3];
}

#endif
