#include "Kaleido3D.h"
#include "Thread.h"
#include <functional>

namespace Concurrency {
	
	Thread::Thread(std::string const & name, Priority priority)
		: m_ThreadName(name)
		, m_ThreadPriority(priority)
		, m_ThreadStatus(Thread::Ready)
	{
	}

	Thread::Thread()
		: Thread("", Thread::Normal)
	{
	}

	Thread::~Thread()
	{
	}

	void Thread::SetPriority(Priority prio)
	{

	}

	void Thread::Start()
	{
	}
	void Thread::Join()
	{
	}
	void Thread::Terminate()
	{
	}
	Thread::Status Thread::GetThreadStatus()
	{
		return m_ThreadStatus;
	}
}