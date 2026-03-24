#include "CoreMinimal.h"
#include <XPlatform/App.h>

namespace k3d
{
    struct EnvironmentImpl
    {
        String InstanceName;
        String ExecutableDir;
        String DataDir;

        EnvironmentImpl()
            : InstanceName("ProsperoApp")
            , ExecutableDir("/app0/sce_module")
            , DataDir("/app0")
        {
        }
    };

    Environment::Environment()
    {
        d = new EnvironmentImpl;
    }

    Environment::~Environment()
    {
        delete d;
    }

    String Environment::GetLogDir() const
    {
        return d->ExecutableDir;
    }

    String Environment::GetModuleDir() const
    {
        return d->ExecutableDir;
    }

    String Environment::GetDataDir() const
    {
        return d->DataDir;
    }

    String Environment::GetInstanceName() const
    {
        return d->InstanceName;
    }
}
