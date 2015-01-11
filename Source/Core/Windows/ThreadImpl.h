#pragma once

#include <Config/OSHeaders.h>
#include "../TaskManager.h"

namespace k3d 
{

	namespace ThreadImpl
	{
		void sleep(uint32 milliSeconds);
		void createThread(IBaseThread & task, TaskPriority priority, WThread::ThreadFunctionPtr functionPtr);
		void createThread(IBaseThread & task, TaskPriority priority, bool deferred, WThread::ThreadFunctionPtr functionPtr);
		void resumeThread(IBaseThread & task);
		void suspendThread(IBaseThread & task);
		void terminateThread(IBaseThread & task);
		void getSysInfo();
	}

}