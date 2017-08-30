#include "Kaleido3D.h"
#include <KTL/String.hpp>
#include <Config/OSHeaders.h>
#include <Core/App.h>
#include <Core/Os.h>

namespace k3d
{
    struct EnvironmentImpl
    {
        String InstanceName;
        String ExecutableDir;
        String DataDir;

        EnvironmentImpl()
            : InstanceName(64)
            , ExecutableDir(400)
            , DataDir(400)
        {
            String ProcessPath(1024);
            readlink("/proc/self/exe", ProcessPath.Data(), 1024);
            ProcessPath.ReCalculate();
            auto Pos = ProcessPath.FindLastOf('/');
            InstanceName = ProcessPath.SubStr(Pos + 1, ProcessPath.Length() - Pos - 1);
            ExecutableDir = ProcessPath.SubStr(0, Pos);
            //DataDir =
            String CurrentPath = ExecutableDir;
            while(!Os::Path::Exists(Os::Path::Join(CurrentPath, "Data").CStr()))
            {
                auto NewPos = CurrentPath.FindLastOf('/');
                if (NewPos == String::npos)
                {
                    DataDir = ".";
                    break;
                }
                CurrentPath = CurrentPath.SubStr(0, NewPos);
                String FindPath = Os::Path::Join(CurrentPath, "Data");
                if (Os::Path::Exists(FindPath.CStr())) {
                    DataDir = FindPath;
                    break;
                }
            }
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

    String Environment::GetDataDir()const
    {
        return d->DataDir;
    }

    String Environment::GetInstanceName() const
    {
        return d->InstanceName;
    }
}
