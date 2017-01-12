#include "MetalRHI.h"
#include "Public/IMetalRHI.h"
#include <vector>

using namespace std;

namespace k3d
{
    class MetalRHI : public IMetalRHI
    {
    public:
        MetalRHI()
        {
        }
        
        ~MetalRHI() override
        {
        }
        
        void Start()override
        {
            KLOG(Info, MetalRHI, "Starting...");
#if K3DPLATFORM_OS_MAC
            NSArray<id<MTLDevice>> * deviceList = MTLCopyAllDevices();
            m_Adapters.resize(deviceList.count);
            MTLFeatureSet fSets[] = {
                MTLFeatureSet_OSX_ReadWriteTextureTier2,
                MTLFeatureSet_OSX_GPUFamily1_v2,
                MTLFeatureSet_OSX_GPUFamily1_v1
            };
            for (uint32 i = 0; i<deviceList.count; i++)
            {
                id<MTLDevice> device = [deviceList objectAtIndex:i];
                m_Adapters[i] = new metal::DeviceAdapter(device);
                KLOG(Info, MetalRHI, "DeviceName: %s ReadWriteTextureTier2:%d GPUv2:%d",
                      [[device name] UTF8String],
                      [device supportsFeatureSet:fSets[0]],
                      [device supportsFeatureSet:fSets[1]]);
            }
#else
            MTLFeatureSet fSets[] = {
                MTLFeatureSet_iOS_GPUFamily2_v1,
                MTLFeatureSet_iOS_GPUFamily1_v2,
                MTLFeatureSet_iOS_GPUFamily2_v2,
                MTLFeatureSet_iOS_GPUFamily3_v1,
                MTLFeatureSet_iOS_GPUFamily1_v3,
                MTLFeatureSet_iOS_GPUFamily2_v3,
                MTLFeatureSet_iOS_GPUFamily3_v2
            };
            id<MTLDevice> device = MTLCreateSystemDefaultDevice();
            m_Adapters.resize(1);
            m_Adapters[0] = new metal::DeviceAdapter(device);
            KLOG(Info, MetalRHI, "DeviceName: %s ReadWriteTextureTier2:%d GPUv2:%d",
                 [[device name] UTF8String],
                 [device supportsFeatureSet:fSets[0]],
                 [device supportsFeatureSet:fSets[1]]);
#endif
        }
        
        void Shutdown()override
        {
            NSLog(@"MetalRHI::Shutdown....");
        }
        
        const char * Name() override { return "RHI_Metal"; }
        
        rhi::DeviceRef GetPrimaryDevice() override
        {
            return m_Adapters[0]->GetDevice();
        }
        
    private:
        vector<metal::DeviceAdapter*> m_Adapters;
    };
}

MODULE_IMPLEMENT(RHI_Metal, k3d::MetalRHI)
