#include "Kaleido3D.h"
#include "ConditionVariable.h"
#include "../LogUtil.h"
#if K3DPLATFORM_OS_WIN
#include <Config/OSHeaders.h>
#endif

namespace Concurrency {
	struct MutexPrivate {
		CRITICAL_SECTION CS;
		MutexPrivate() {
			InitializeCriticalSection(&CS);
		}
		void Lock() {
			EnterCriticalSection(&CS);
		}
		void UnLock() {
			LeaveCriticalSection(&CS);
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
		CONDITION_VARIABLE CV;
		ConditionVariablePrivate() {
			InitializeConditionVariable(&CV);
		}
		~ConditionVariablePrivate() {
		}
		void Wait(MutexPrivate * mutex, UINT time)
		{
			::SleepConditionVariableCS(&CV, &(mutex->CS), time);
		}
		void Notify() {
			::WakeConditionVariable(&CV);
		}
		void NotifyAll() {
			::WakeAllConditionVariable(&CV);
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