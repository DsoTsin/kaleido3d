#include "Kaleido3D.h"
#include "Thread.h"

#include <functional>
#include "Windows.h"
#include "ConditionVariable.h"

#ifdef Yield
#undef Yield
#endif
#define DEFAULT_THREAD_STACK_SIZE 0

namespace Concurrency {

	std::map<uint32, Thread*> Thread::s_ThreadMap;
	
	Thread::Thread(std::string const & name, ThreadPriority priority)
		: m_ThreadName(name)
		, m_ThreadPriority(priority)
		, m_StackSize(DEFAULT_THREAD_STACK_SIZE)
		, m_ThreadStatus(ThreadStatus::Ready)
		, m_ThreadHandle(nullptr)
	{
	}

	void Thread::SleepForMilliSeconds(uint32_t millisecond)
	{
		::Sleep(millisecond);
	}

	/*void Thread::Yield()
	{
		::Sleep(0);
	}*/

	uint32_t Thread::GetId()
	{
		return ::GetCurrentThreadId();
	}

	Thread::Thread()
		: Thread("", ThreadPriority::Normal)
	{
	}

	Thread::Thread(
		Call && callback,
		std::string const & name,
		ThreadPriority priority
		)
		: m_ThreadCallBack(std::forward<Call>(callback))
		, m_ThreadName(name)
		, m_ThreadPriority(priority)
		, m_StackSize(DEFAULT_THREAD_STACK_SIZE)
		, m_ThreadStatus(ThreadStatus::Ready)
		, m_ThreadHandle(nullptr)
	{
	}

	Thread::~Thread()
	{
	}

	void Thread::SetPriority(ThreadPriority prio)
	{
		m_ThreadPriority = prio;
	}

	void Thread::Start()
	{
#if defined(K3DPLATFORM_OS_WIN) || defined(_WIN32)
		if (nullptr == m_ThreadHandle)
		{
			m_ThreadHandle = ::CreateThread(nullptr, m_StackSize, reinterpret_cast<LPTHREAD_START_ROUTINE>(Run), reinterpret_cast<LPVOID>(/*&std::make_shared<Thread>*/(this)), 0, nullptr);
			{
				Concurrency::Mutex::AutoLock lock;
				DWORD tid = ::GetThreadId(m_ThreadHandle);
				s_ThreadMap[tid] = this;
			}
		}
#endif
	}


	void Thread::Join()
	{
		if (m_ThreadHandle != nullptr) {
			::WaitForSingleObject(m_ThreadHandle, INFINITE);
		}
	}

	void Thread::Terminate()
	{
		if (m_ThreadHandle != nullptr) {
			::TerminateThread(m_ThreadHandle, 0);
		}
	}

	ThreadStatus Thread::GetThreadStatus()
	{
		return m_ThreadStatus;
	}

	std::string Thread::GetName()
	{
		return m_ThreadName;
	}

	std::string  Thread::GetCurrentThreadName() {
		uint32 tid = (uint32)::GetCurrentThreadId();
		if (s_ThreadMap[tid] != nullptr) {
			return s_ThreadMap[tid]->GetName();
		}
		return "Anonymous Thread";
	}

	void Thread::Run(void *data)
	{
		Thread* thr = reinterpret_cast<Thread*>(data);
		if (thr != nullptr)
		{
			Call call = thr->m_ThreadCallBack;
			call();
			thr->m_ThreadStatus = ThreadStatus::Finish;
			::ExitThread(0);
		}
	}
}