#include "kVehicleSceneQuery.h"
#include "PsAllocator.h"

#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));

void SampleVehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0==qryFilterData->word3 && "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_DRIVABLE_SURFACE;
}

void SampleVehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0==qryFilterData->word3 && "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData)
{
    assert(0==qryFilterData->word3 && "word3 is reserved for filter data for vehicle raycast queries");
    qryFilterData->word3 = (PxU32)SAMPLEVEHICLE_UNDRIVABLE_SURFACE;
}

SampleVehicleSceneQueryData* SampleVehicleSceneQueryData::allocate(const PxU32 maxNumWheels)
{
    const PxU32 size0 = SIZEALIGN16(sizeof(SampleVehicleSceneQueryData));
    const PxU32 size1 = SIZEALIGN16(sizeof(PxRaycastQueryResult)*maxNumWheels);
    const PxU32 size2 = SIZEALIGN16(sizeof(PxRaycastHit)*maxNumWheels);
    const PxU32 size = size0 + size1 + size2;
    SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)PX_ALLOC(size, PX_DEBUG_EXP("PxVehicleNWSceneQueryData"));
    sqData->init();
    PxU8* ptr = (PxU8*) sqData;
    ptr += size0;
    sqData->mSqResults = (PxRaycastQueryResult*)ptr;
    sqData->mNbSqResults = maxNumWheels;
    ptr += size1;
    sqData->mSqHitBuffer = (PxRaycastHit*)ptr;
    ptr += size2;
    sqData->mNumQueries = maxNumWheels;
    return sqData;
}

void SampleVehicleSceneQueryData::free()
{
    PX_FREE(this);
}

PxBatchQuery* SampleVehicleSceneQueryData::setUpBatchedSceneQuery(PxScene* scene)
{
    PxBatchQueryDesc sqDesc(mNbSqResults, 0, 0);
    sqDesc.queryMemory.userRaycastResultBuffer = mSqResults;
    sqDesc.queryMemory.userRaycastTouchBuffer = mSqHitBuffer;
    sqDesc.queryMemory.raycastTouchBufferSize = mNumQueries;
    sqDesc.preFilterShader = mPreFilterShader;
    sqDesc.spuPreFilterShader = mSpuPreFilterShader;
    sqDesc.spuPreFilterShaderSize = mSpuPreFilterShaderSize;
    return scene->createBatchQuery(sqDesc);
}
