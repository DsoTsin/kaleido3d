#pragma once
#ifndef __TaskManager_h__
#define __TaskManager_h__
#include <Interface/IRunnable.h>
#include <KTL/Singleton.hpp>
#include <KTL/ThreadPool.hpp>
#include "Dispatch/WorkQueue.h"

namespace Concurrency {

	//using ThreadPool = std::thread_pool;
	using ThreadSinglePool = std::single_thread_pool;

	class TaskWrapper2 : public Dispatch::WorkQueue {
	public:
		TaskWrapper2();
		~TaskWrapper2();
	};
}

#endif
