#include "Kaleido3D.h"
#include "ConditionVariable.h"
#include <Config/OSHeaders.h>

namespace Concurrency {
	struct MutexPrivate {
#if K3DPLATFORM_OS_WIN
		CRITICAL_SECTION CS;
#else
		pthread_mutex_t mMutex;
#endif
		MutexPrivate() {
#if K3DPLATFORM_OS_WIN
			InitializeCriticalSection(&CS);
#else
			pthread_mutex_init(&mMutex, NULL);
#endif
		}

		~MutexPrivate() {
#if K3DPLATFORM_OS_WIN
//			InitializeCriticalSection(&CS);
#else
			pthread_mutex_destroy(&mMutex);
#endif
		}

		void Lock() {
#if K3DPLATFORM_OS_WIN
			EnterCriticalSection(&CS);
#else
            pthread_mutex_lock(&mMutex);
#endif
		}
		void UnLock() {
#if K3DPLATFORM_OS_WIN
			LeaveCriticalSection(&CS);
#else
			pthread_mutex_unlock(&mMutex);
#endif
		}
	};

	Mutex::Mutex():m_Impl(new MutexPrivate)
	{
	}

	Mutex::~Mutex()
	{
		delete m_Impl;
	}

	void Mutex::Lock()
	{
		m_Impl->Lock();
	}

	void Mutex::UnLock()
	{
		m_Impl->UnLock();
	}

	struct ConditionVariablePrivate {
#if K3DPLATFORM_OS_WIN
		CONDITION_VARIABLE CV;
#else
		pthread_cond_t mCond;
#endif
		ConditionVariablePrivate() {
#if K3DPLATFORM_OS_WIN
			InitializeConditionVariable(&CV);
#else
			pthread_cond_init(&mCond, NULL);
#endif
		}
		~ConditionVariablePrivate() {
#if K3DPLATFORM_OS_WIN
#else
			pthread_cond_destroy(&mCond);
#endif
		}

		void Wait(MutexPrivate * mutex, uint32 time)
		{
#if K3DPLATFORM_OS_WIN
			::SleepConditionVariableCS(&CV, &(mutex->CS), time);
#else
			pthread_cond_wait(&mCond, &mutex->mMutex);
#endif
		}
		void Notify() {
#if K3DPLATFORM_OS_WIN
			::WakeConditionVariable(&CV);
#else
			pthread_cond_signal(&mCond);
#endif
		}
		void NotifyAll() {
#if K3DPLATFORM_OS_WIN
			::WakeAllConditionVariable(&CV);
#else
			pthread_cond_broadcast(&mCond);
#endif
		}
	};

	ConditionVariable::ConditionVariable() : m_Impl(new ConditionVariablePrivate)
	{
	}

	ConditionVariable::~ConditionVariable()
	{
		delete m_Impl;
	}

	void ConditionVariable::Wait(Mutex * mutex)
	{
		if(mutex!=nullptr)
			m_Impl->Wait(mutex->m_Impl, 0xffffffff);
	}

	void ConditionVariable::Wait(Mutex * mutex, uint32 milliseconds)
	{
		if (mutex != nullptr)
			m_Impl->Wait(mutex->m_Impl, milliseconds);
	}

	void ConditionVariable::Notify()
	{
		m_Impl->Notify();
	}

	void ConditionVariable::NotifyAll()
	{
		m_Impl->NotifyAll();
	}


}