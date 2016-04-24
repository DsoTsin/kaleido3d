#pragma once 
#include "ThreadCommon.h"
#include "Kaleido3D.h"
#include <functional>

namespace Concurrency {

	class K3D_API Thread {
	public:
		// static functions
		static void SleepForMilliSeconds(uint32_t millisecond);
		//static void Yield();
		static uint32_t GetId();

	public:
		typedef void * Handle;
		typedef std::function<void()> Call;

		Thread();

		explicit		Thread(std::string const & name, ThreadPriority priority = ThreadPriority::Normal);
		explicit		Thread(Call && callback, std::string const & name, ThreadPriority priority = ThreadPriority::Normal);
		
		virtual			~Thread();

		void			SetPriority(ThreadPriority prio);
		void			Start();
		void			Join();
		void			Terminate();

		ThreadStatus	GetThreadStatus();
		std::string		GetName();
	
	public:
		static std::string  GetCurrentThreadName();

	private:
		Call				m_ThreadCallBack;
		std::string         m_ThreadName;
		ThreadPriority	    m_ThreadPriority;
		uint32_t			m_StackSize;
		ThreadStatus		m_ThreadStatus;
		Handle				m_ThreadHandle;

	private:

		static void* STD_CALL Run(void*);
		static std::map<uint32, Thread*>	s_ThreadMap;

	};
}