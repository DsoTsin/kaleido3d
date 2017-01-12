#pragma once
#include <Physics/PxSupport.h>

enum
{
    DRIVABLE_SURFACE = 0xffff0000,
    UNDRIVABLE_SURFACE = 0x0000ffff
};

void setupDrivableSurface(PxFilterData& filterData);

void setupNonDrivableSurface(PxFilterData& filterData);

PxQueryHitType::Enum WheelRaycastPreFilter
(PxFilterData filterData0, PxFilterData filterData1,
 const void* constantBlock, PxU32 constantBlockSize,
 PxHitFlags& queryFlags);

class VehicleSceneQueryData
{
public:
    VehicleSceneQueryData();
    ~VehicleSceneQueryData();

    //Allocate scene query data for up to maxNumVehicles and up to maxNumWheelsPerVehicle with numVehiclesInBatch per batch query.
    static VehicleSceneQueryData* allocate(
        const PxU32 maxNumVehicles,
        const PxU32 maxNumWheelsPerVehicle,
        const PxU32 numVehiclesInBatch,
        PxAllocatorCallback& allocator);

    //Free allocated buffers.
    void free(PxAllocatorCallback& allocator);

    //Create a PxBatchQuery instance that will be used for a single specified batch.
    static PxBatchQuery* setUpBatchedSceneQuery(
        const PxU32 batchId,
        const VehicleSceneQueryData&
        vehicleSceneQueryData,
        PxScene* scene);

    //Return an array of scene query results for a single specified batch.
    PxRaycastQueryResult* getRaycastQueryResultBuffer(const PxU32 batchId);

    //Get the number of scene query results that have been allocated for a single batch.
    PxU32 getRaycastQueryResultBufferSize() const;

private:

    //Number of raycasts per batch
    PxU32 mNumRaycastsPerBatch;

    //One result for each wheel.
    PxRaycastQueryResult* mSqResults;

    //One hit for each wheel.
    PxRaycastHit* mSqHitBuffer;

    //Filter shader used to filter drivable and non-drivable surfaces
    PxBatchQueryPreFilterShader mPreFilterShader;
};
