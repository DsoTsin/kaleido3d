#include "ThreadImpl.h"

#import <Foundation/NSThread.h>


namespace k3d {
    
    namespace ThreadImpl
    {
        void getSysInfo()
        {
            
        }
        
        void createThread(IBaseThread & task, TaskPriority priority, WThread::ThreadFunctionPtr func)
        {
            task.m_Handle = [[NSThread alloc] init];
            NSThread * instance = (NSThread*)task.m_Handle;
            [instance setThreadPriority:0.0];
            [instance performSelector:@selector(func)];
            [instance start];
        }
        
        void createThread(IBaseThread & task, TaskPriority priority, bool deferred, WThread::ThreadFunctionPtr func)
        {
            task.m_Handle = [[NSThread alloc] init];
            NSThread * instance = (NSThread*)task.m_Handle;
            [instance setThreadPriority:0.0];
            [instance performSelector:@selector(func)];
            if(!deferred)
                [instance start];
        }
        
        void sleep(uint32 milliSeconds)
        {
            [NSThread sleepForTimeInterval:milliSeconds];
        }
        
        void suspendThread(IBaseThread & task)
        {
            NSThread* instance = (NSThread*)task.m_Handle;
            [instance cancel];
        }
        
        void terminateThread(IBaseThread & task)
        {
            NSThread* instance = (NSThread*)task.m_Handle;
            [instance cancel];
        }
        
        void resumeThread(IBaseThread & task)
        {
            NSThread* instance = (NSThread*)task.m_Handle;
            [instance start];
        }
    }
        
}