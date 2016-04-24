#include "Kaleido3D.h"
#include "Thread.h"

#include <functional>
#include <Config/OSHeaders.h>
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
#if K3DPLATFORM_OS_WIN
		::Sleep(millisecond);
#else
		::usleep(millisecond*1000);
#endif
	}

	/*void Thread::Yield()
	{
		::Sleep(0);
	}*/

	uint32_t Thread::GetId()
	{
#if K3DPLATFORM_OS_WIN
		return ::GetCurrentThreadId();
#else
		return (uint32_t)pthread_self();
#endif
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
#else
        if(0 == (u_long)m_ThreadHandle)
		{
			typedef void* (threadfun)(void*);
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_create((pthread_t*)&m_ThreadHandle, nullptr, Run, this);
		}
#endif
	}


	void Thread::Join()
	{
		if (m_ThreadHandle != nullptr) {
#if K3DPLATFORM_OS_WIN
			::WaitForSingleObject(m_ThreadHandle, INFINITE);
#else
            void* ret;
            pthread_join((pthread_t)m_ThreadHandle, &ret);
#endif
		}
	}

	void Thread::Terminate()
	{
		if (m_ThreadHandle != nullptr) {
#if K3DPLATFORM_OS_WIN
			::TerminateThread(m_ThreadHandle, 0);
#endif
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
#if K3DPLATFORM_OS_WIN
		uint32 tid = (uint32)::GetCurrentThreadId();
#else
        uint32 tid = (uint32)pthread_self();
#endif
		if (s_ThreadMap[tid] != nullptr) {
			return s_ThreadMap[tid]->GetName();
		}
		return "Anonymous Thread";
	}

	void* Thread::Run(void *data)
	{
		Thread* thr = reinterpret_cast<Thread*>(data);
		if (thr != nullptr)
		{
			Call call = thr->m_ThreadCallBack;
			call();
			thr->m_ThreadStatus = ThreadStatus::Finish;
#if K3DPLATFORM_OS_WIN
			::ExitThread(0);
#else
            int ret = 0;
            pthread_exit(&ret);
#endif
		}
		return thr;
	}
}