#include "Kaleido3D.h"

#include "ThreadPool.h"

namespace Concurrency {
	ThreadPool::ThreadPool(
		std::string const poolName, 
		uint32_t capacity, 
		::Concurrency::ThreadPriority priority
		)
		: m_WorkerThreads(nullptr)
		, m_WorkerCount(capacity)
	{
		m_WorkerThreads = new Thread[m_WorkerCount];
	}

}