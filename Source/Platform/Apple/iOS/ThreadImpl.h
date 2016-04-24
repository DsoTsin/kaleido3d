#pragma once
#include <Kaleido3D.h>
#include "../TaskManager.h"

namespace k3d {

    namespace ThreadImpl
    {
        void getSysInfo();
        void createThread(IBaseThread & task, TaskPriority priority, WThread::ThreadFunctionPtr func);
        void createThread(IBaseThread & task, TaskPriority priority, bool deferred, WThread::ThreadFunctionPtr func);
        void sleep(uint32 milliSeconds);
        void terminateThread(IBaseThread &);
        void suspendThread(IBaseThread &);
        void resumeThread(IBaseThread &);
    }
    
}