#pragma once 
#include "Thread.h"

namespace Concurrency {
	class ThreadPool {
	public:
		ThreadPool(std::string const poolName, uint32_t capacity, ::Concurrency::ThreadPriority priority);


	private:
		Thread*		m_WorkerThreads;
		uint32_t	m_WorkerCount;
	};
}