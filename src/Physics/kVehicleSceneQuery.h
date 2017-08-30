#pragma once
#include "kPsEngine.h"

//Make sure that suspension raycasts only consider shapes flagged as drivable that don't belong to the owner vehicle.
enum
{
  SAMPLEVEHICLE_DRIVABLE_SURFACE = 0xffff0000,
  SAMPLEVEHICLE_UNDRIVABLE_SURFACE = 0x0000ffff
};

static PxSceneQueryHitType::Enum SampleVehicleWheelRaycastPreFilter(
    PxFilterData filterData0,
    PxFilterData filterData1,
    const void* constantBlock, PxU32 constantBlockSize,
    PxSceneQueryFlags& queryFlags)
{
  //filterData0 is the vehicle suspension raycast.
  //filterData1 is the shape potentially hit by the raycast.
  PX_UNUSED(queryFlags);
  PX_UNUSED(constantBlockSize);
  PX_UNUSED(constantBlock);
  PX_UNUSED(filterData0);
  return ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE)) ? PxSceneQueryHitType::eNONE : PxSceneQueryHitType::eBLOCK);
}


//Set up query filter data so that vehicles can drive on shapes with this filter data.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void SampleVehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData);

//Set up query filter data so that vehicles cannot drive on shapes with this filter data.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void SampleVehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData);

//Set up query filter data for the shapes of a vehicle to ensure that vehicles cannot drive on themselves
//but can drive on the shapes of other vehicles.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData);

//Data structure for quick setup of scene queries for suspension raycasts.
class SampleVehicleSceneQueryData
{
public:

  //Allocate scene query data for up to maxNumWheels suspension raycasts.
  static SampleVehicleSceneQueryData* allocate(const PxU32 maxNumWheels);

  //Free allocated buffer for scene queries of suspension raycasts.
  void free();

  //Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
  PxBatchQuery* setUpBatchedSceneQuery(PxScene* scene);

  //Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
  PxRaycastQueryResult* getRaycastQueryResultBuffer() {return mSqResults;}

  //Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
  PxU32 getRaycastQueryResultBufferSize() const {return mNumQueries;}

  //Set the pre-filter shader
  void setPreFilterShader(PxBatchQueryPreFilterShader preFilterShader) {mPreFilterShader=preFilterShader;}

  //Set the spu pre-filter shader (essential to run filtering on spu)
  void setSpuPreFilterShader(void* spuPreFilterShader, const PxU32 spuPreFilterShaderSize) {mSpuPreFilterShader=spuPreFilterShader; mSpuPreFilterShaderSize=spuPreFilterShaderSize;}

private:

  //One result for each wheel.
  PxRaycastQueryResult* mSqResults;
  PxU32 mNbSqResults;

  //One hit for each wheel.
  PxRaycastHit* mSqHitBuffer;

  //Filter shader used to filter drivable and non-drivable surfaces
  PxBatchQueryPreFilterShader mPreFilterShader;

  //Ptr to compiled spu filter shader
  //Set this on ps3 for spu raycasts
  void* mSpuPreFilterShader;

  //Size of compiled spu filter shader
  //Set this on ps3 for spu raycasts.
  PxU32 mSpuPreFilterShaderSize;

  //Maximum number of suspension raycasts that can be supported by the allocated buffers
  //assuming a single query and hit per suspension line.
  PxU32 mNumQueries;

  void init()
  {
    mPreFilterShader=SampleVehicleWheelRaycastPreFilter;
    mSpuPreFilterShader=NULL;
    mSpuPreFilterShaderSize=0;
  }

  SampleVehicleSceneQueryData()
  {
    init();
  }

  ~SampleVehicleSceneQueryData()
  {
  }
};

