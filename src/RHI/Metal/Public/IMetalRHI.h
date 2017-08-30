#pragma once 
#include <Core/Module.h>
#include <Core/Interface/IRHI.h>

namespace k3d
{
    class IMetalRHI : public IModule
    {
    public:
        virtual NGFXDeviceRef	GetPrimaryDevice() = 0;
    };
}

#if BUILD_STATIC_PLUGIN
K3D_STATIC_MODULE_DECLARE(RHI_Metal);
#endif
