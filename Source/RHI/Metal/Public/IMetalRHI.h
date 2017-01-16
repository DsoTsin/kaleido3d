#pragma once 
#include <Core/Module.h>
#include <Interface/IRHI.h>

namespace k3d
{
    class IMetalRHI : public IModule
    {
    public:
        virtual rhi::DeviceRef	GetPrimaryDevice() = 0;
    };
}

#if BUILD_STATIC_PLUGIN
K3D_STATIC_MODULE_DECLARE(RHI_Metal);
#endif
