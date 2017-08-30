#include "kVehicleRaycast.h"

void setupDrivableSurface(PxFilterData& filterData)
{
    filterData.word3 = (PxU32)DRIVABLE_SURFACE;
}

void setupNonDrivableSurface(PxFilterData& filterData)
{
    filterData.word3 = UNDRIVABLE_SURFACE;
}

PxQueryHitType::Enum WheelRaycastPreFilter
(PxFilterData filterData0, PxFilterData filterData1,
 const void* constantBlock, PxU32 constantBlockSize,
 PxHitFlags& queryFlags)
{
    //filterData0 is the vehicle suspension raycast.
    //filterData1 is the shape potentially hit by the raycast.
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);
    PX_UNUSED(filterData0);
    PX_UNUSED(queryFlags);
    return ((0 == (filterData1.word3 & DRIVABLE_SURFACE)) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK);
}

VehicleSceneQueryData::VehicleSceneQueryData()
:  mNumRaycastsPerBatch(0),
   mSqResults(NULL),
   mSqHitBuffer(NULL),
   mPreFilterShader(WheelRaycastPreFilter)
{
}

VehicleSceneQueryData::~VehicleSceneQueryData()
{
}

VehicleSceneQueryData* VehicleSceneQueryData::allocate(const PxU32 maxNumVehicles, const PxU32 maxNumWheelsPerVehicle, const PxU32 numVehiclesInBatch, PxAllocatorCallback& allocator)
{
    PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxRaycastQueryResult) & 15));
    PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxRaycastHit) & 15));

    const PxU32 sqDataSize = ((sizeof(VehicleSceneQueryData) + 15) & ~15);

    const PxU32 maxNumWheels = maxNumVehicles*maxNumWheelsPerVehicle;

    const PxU32 size = sqDataSize + sizeof(PxRaycastQueryResult)*maxNumWheels + sizeof(PxRaycastHit)*maxNumWheels;
    PxU8* buffer = (PxU8*)allocator.allocate(size, NULL, NULL, 0);

    VehicleSceneQueryData* sqData = new(buffer) VehicleSceneQueryData();
    buffer += sqDataSize;

    sqData->mNumRaycastsPerBatch = numVehiclesInBatch*maxNumWheelsPerVehicle;

    sqData->mSqResults = (PxRaycastQueryResult*)buffer;
    buffer += sizeof(PxRaycastQueryResult)*maxNumWheels;

    sqData->mSqHitBuffer = (PxRaycastHit*)buffer;
    buffer += sizeof(PxRaycastHit)*maxNumWheels;

    for(PxU32 i = 0; i < maxNumVehicles; i++)
    {
        for(PxU32 j = 0; j < maxNumWheelsPerVehicle; j++)
        {
            PX_ASSERT((size_t)(sqData->mSqResults + i*maxNumWheelsPerVehicle + j) < (size_t)buffer);
            PX_ASSERT((size_t)(sqData->mSqHitBuffer + i*maxNumWheelsPerVehicle + j) < (size_t)buffer);
            new(sqData->mSqResults + i*maxNumWheelsPerVehicle + j) PxRaycastQueryResult();
            new(sqData->mSqHitBuffer + i*maxNumWheelsPerVehicle + j) PxRaycastHit();
        }
    }

    return sqData;
}

void VehicleSceneQueryData::free(PxAllocatorCallback& allocator)
{
    allocator.deallocate(this);
}

PxBatchQuery* VehicleSceneQueryData::setUpBatchedSceneQuery(const PxU32 batchId, const VehicleSceneQueryData& vehicleSceneQueryData, PxScene* scene)
{
    const PxU32 maxNumRaycastsInBatch =  vehicleSceneQueryData.mNumRaycastsPerBatch;
    PxBatchQueryDesc sqDesc(maxNumRaycastsInBatch, 0, 0);
    sqDesc.queryMemory.userRaycastResultBuffer = vehicleSceneQueryData.mSqResults + batchId*maxNumRaycastsInBatch;
    sqDesc.queryMemory.userRaycastTouchBuffer = vehicleSceneQueryData.mSqHitBuffer + batchId*maxNumRaycastsInBatch;
    sqDesc.queryMemory.raycastTouchBufferSize = maxNumRaycastsInBatch;
    sqDesc.preFilterShader = vehicleSceneQueryData.mPreFilterShader;
    return scene->createBatchQuery(sqDesc);
}

PxRaycastQueryResult* VehicleSceneQueryData::getRaycastQueryResultBuffer(const PxU32 batchId)
{
    return (mSqResults + batchId*mNumRaycastsPerBatch);
}

PxU32 VehicleSceneQueryData::getRaycastQueryResultBufferSize() const
{
    return mNumRaycastsPerBatch;
}
