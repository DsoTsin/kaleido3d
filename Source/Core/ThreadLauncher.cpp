#include "Kaleido3D.h"
#include "ThreadLauncher.h"
#include "KTL/ThreadPool.hpp"
#include <Config/OSHeaders.h>

namespace Concurrency {

#if defined(K3DPLATFORM_OS_WIN)
	inline static INT GetThreadPriority(ThreadPriority & priority) {
		INT winPriority = THREAD_PRIORITY_NORMAL;
		switch (priority) {
		case ThreadPriority::RealTime:
			winPriority = THREAD_PRIORITY_HIGHEST;
			break;
		case ThreadPriority::High:
			winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case ThreadPriority::Low:
			winPriority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case ThreadPriority::Normal:
		default:
			break;
		}
		return winPriority;
	}
#endif

	void ThreadLauncherBase::Launch(ThreadInternal * thr, ThreadPriority priority)
	{
#if defined(K3DPLATFORM_OS_WIN)
		void *data = malloc(sizeof(*this));
		memcpy(data, this, sizeof(*this));
		thr->handle = ::CreateThread(NULL, 1024, (LPTHREAD_START_ROUTINE)&CallFunc, (LPVOID)data, 0, NULL);
#if _DEBUG
		if (::SetThreadPriority(thr->handle, GetThreadPriority(priority)) == TRUE) {
			OutputDebugStringA("::SetThreadPriority succeed.\n");
		}
#endif
#endif
		while (!m_Started)
			std::_Cnd_waitX(m_Cond, m_Mtx);
	}

	ThreadLauncherBase::ThreadLauncherBase()
	{
		std::_Cnd_initX(&m_Cond);
		std::_Auto_cnd _Cnd_cleaner(m_Cond);
		std::_Mtx_initX(&m_Mtx, _Mtx_plain);
		std::_Auto_mtx _Mtx_cleaner(m_Mtx);
		m_Started = false;
		std::_Mtx_lockX(m_Mtx);
		_Mtx_cleaner._Release();
		_Cnd_cleaner._Release();
	}
	
	ThreadLauncherBase::~ThreadLauncherBase()
	{
		std::_Auto_cnd _Cnd_cleaner(m_Cond);
		std::_Auto_mtx _Mtx_cleaner(m_Mtx);
		std::_Mtx_unlockX(m_Mtx);
	}

	void ThreadLauncherBase::Release()
	{
		std::_Mtx_lockX(m_Mtx);
		m_Started = true;
		std::_Cnd_signalX(m_Cond);
		std::_Mtx_unlockX(m_Mtx);
	}

	uint32 ThreadLauncherBase::CallFunc(void*data)
	{
		static_cast<ThreadLauncherBase *>(data)->Go();
		::ExitThread(0);
		return (0);
	}
}