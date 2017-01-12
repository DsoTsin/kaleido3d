#pragma once 
#include <Core/Module.h>
#include <RHI/IRHI.h>

namespace k3d
{
    class IMetalRHI : public IModule
    {
    public:
        virtual rhi::DeviceRef	GetPrimaryDevice() = 0;
    };
}
