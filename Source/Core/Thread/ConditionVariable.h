#pragma once
#include "Kaleido3D.h"

namespace Concurrency {

	enum class WaitResult {
		TimeOut,
		Failed,
		Success
	};

	struct MutexPrivate;
	class K3D_API Mutex 
	{
	public:
		Mutex();
		~Mutex();

		void Lock();
		void UnLock();
		friend class ConditionVariable;

		struct AutoLock 
		{
			AutoLock() 
			{
				m_Mutex = new Mutex;
				m_Mutex->Lock();
			}

			explicit AutoLock(Mutex * mutex, bool lostOwnerShip = false) 
				: m_OnwerShipGot(lostOwnerShip)
				, m_Mutex(mutex) 
			{
			}

			~AutoLock() 
			{
				m_Mutex->UnLock();
				if(m_OnwerShipGot) 
				{
					delete m_Mutex;
				}
			}
		private:
			bool	m_OnwerShipGot;
			Mutex	*m_Mutex;
		};

	private:
		MutexPrivate * m_Impl;
	};


	struct ConditionVariablePrivate;
	class K3D_API ConditionVariable {
	public:
		ConditionVariable();
		~ConditionVariable();

		void Wait(Mutex * mutex);
		void Wait(Mutex * mutex, uint32 milliseconds);
		void Notify();
		void NotifyAll();

		ConditionVariable(const ConditionVariable &) = delete;
		ConditionVariable(const ConditionVariable &&) = delete;

	protected:

		ConditionVariablePrivate * m_Impl;
	};


}