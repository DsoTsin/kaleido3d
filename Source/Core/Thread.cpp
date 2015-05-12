#include "Kaleido3D.h"
#include "Thread.h"
#include "ThreadLauncher.h"

#include <functional>
#include <Config/OSHeaders.h>

namespace Concurrency {

	Thread::Thread(std::string const & name, ThreadPriority priority)
		: m_ThreadName(name)
		, m_ThreadPriority(priority)
		, m_ThreadStatus(ThreadStatus::Ready)
		, m_ThreadInternal(new ThreadInternal)
	{
	}

	Thread::Thread()
		: Thread("", ThreadPriority::Normal)
	{
	}

	Thread::~Thread()
	{
	}

	void Thread::SetPriority(ThreadPriority prio)
	{

	}

	void Thread::Start()
	{
#if defined(K3DPLATFORM_OS_WIN)
		if (m_ThreadInternal != nullptr && m_ThreadInternal->IsValid())
		{
			::ResumeThread(m_ThreadInternal->handle);
		}
#endif
	}


	void Thread::Join()
	{
		if (m_ThreadInternal != nullptr && m_ThreadInternal->IsValid()) {
			::WaitForSingleObject(m_ThreadInternal->handle, INFINITE);
		}
	}

	void Thread::Terminate()
	{
	}

	ThreadStatus Thread::GetThreadStatus()
	{
		return m_ThreadStatus;
	}
}