#include "CoreMinimal.h"
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#include <sys/types.h>
#include <sys/sysctl.h>

namespace k3d
{

struct __CPU_Ticks
{
    uint64 usertime;
    uint64 nicetime;
    uint64 systemtime;
    uint64 idletime;
    
    uint64 used() { return usertime + nicetime + systemtime; }
    uint64 total() { return usertime + nicetime + systemtime + idletime; }
};

static k3d::DynArray<float> usages;
static k3d::DynArray<__CPU_Ticks> ticks;
static k3d::DynArray<__CPU_Ticks> curTicks;
static int cpuCount = 0;

int __UpdateCpuUsage(k3d::DynArray<__CPU_Ticks> & ticks)
{
    unsigned int cpu_count;
    processor_cpu_load_info_t cpu_load;
    mach_msg_type_number_t cpu_msg_count;
    int rc =  host_processor_info(
                                  mach_host_self( ),
                                  PROCESSOR_CPU_LOAD_INFO,
                                  &cpu_count,
                                  (processor_info_array_t *) &cpu_load,
                                  &cpu_msg_count
                                  );
    
    if (rc != 0)
        return 1;
    
    for(int core=0; core<cpuCount; core++)
    {
        if (core < 0 || cpu_count <= core)
            break;
        uint64 usertime = cpu_load[core].cpu_ticks[CPU_STATE_USER];
        uint64 nicetime = cpu_load[core].cpu_ticks[CPU_STATE_NICE];
        uint64 systemtime = cpu_load[core].cpu_ticks[CPU_STATE_SYSTEM];
        uint64 idletime = cpu_load[core].cpu_ticks[CPU_STATE_IDLE];
        ticks[core] = {usertime, nicetime, systemtime, idletime};
    }
    return 0;
}

int __Init(void)
{
    cpuCount = k3d::os::GetCpuCoreNum();
    usages.Resize(cpuCount);
    ticks.Resize(cpuCount);
    curTicks.Resize(cpuCount);
    __UpdateCpuUsage(ticks);
    return 0;
}

float* __GetCpuUsage()
{
    if(cpuCount==0)
    {
        __Init();
    }
    __UpdateCpuUsage(curTicks);
    for(int i=0;i<cpuCount;i++)
    {
        auto& prev = ticks[i];
        auto t = curTicks[i];
        uint64 used = t.used() - prev.used();
        uint64 total = t.total() - prev.total();
        prev = t;
        usages[i] = (float)used / (float)total * 100.0f;
    }
    return usages.Data();
}


namespace os
{
    int GetCpuCount()
    {
        static I32 numCores = -1;
        if(numCores == -1)
        {
            size_t size = sizeof(I32);
            if(sysctlbyname("hw.ncpu", &numCores, &size, NULL, 0) != 0)
            {
                numCores = 1;
            }
        }
        return numCores;
    }
    
    U32 GetGpuCount()
    {
        return 1;
    }
    
    float* GetCpuUsage()
    {
        return __GetCpuUsage();
    }
    
} // os
    
} // k3d
