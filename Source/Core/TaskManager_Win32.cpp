#include "TaskManager.h"

#include <Windows.h>
#include <strsafe.h>

namespace k3d {

	static void __stdcall _createTask(IBaseThread * task) {
		if (task) {
			task->OnRun();
		}
	}

	void TaskManager::initManager() 
	{
		SYSTEM_INFO info;
		::GetSystemInfo(&info);

		TCHAR msgBuf[1024] = { 0 };
		::StringCchPrintf(msgBuf, 1024, TEXT("Avaliable CPU Core Num= %d\n"), info.dwNumberOfProcessors);
		::OutputDebugString(msgBuf);
	}

	void TaskManager::createThread(IBaseThread * task, TaskPriority priority)
	{
		task->m_Handle = ::CreateThread(
			NULL,
			NULL,
			(LPTHREAD_START_ROUTINE)_createTask,
			task,
			0, /* 0,CREATE_SUSPENDED,STACK_SIZE_PARAM_IS_A_RESERVATION */
			NULL);

		if (task->m_Handle != nullptr) {
			int _tPriority = THREAD_PRIORITY_NORMAL;
			switch (priority) {
			case TaskPriority::RealTime:
				_tPriority = THREAD_PRIORITY_HIGHEST;
				break;
			case TaskPriority::BackGround:
				_tPriority = THREAD_PRIORITY_BELOW_NORMAL;
			case TaskPriority::Normal:
			default:
				break;
			}
#if _DEBUG
			BOOL res =
#endif
				::SetThreadPriority(task->m_Handle, _tPriority);
#if _DEBUG
			if (res == TRUE) {
				OutputDebugStringA("::SetThreadPriority succeed.\n");
			}
#endif
			m_ThreadQueue.push(task);
		}
	}

	void TaskManager::resumeThread(IBaseThread * task)
	{
		if (task) {
			::ResumeThread(task->m_Handle);
		}
	}

	void TaskManager::suspendThread(IBaseThread * task)
	{
		if (task) {
			::SuspendThread(task->m_Handle);
		}
	}

	void TaskManager::terminateThread(IBaseThread * task)
	{
		if (task && task->m_Handle) {
			DWORD exCode = 0;
			::TerminateThread(task->m_Handle, exCode);
#if _DEBUG
			TCHAR msgBuf[1024] = { 0 };
			::StringCchPrintf(msgBuf, 1024, TEXT("exit code= %d\n"), exCode);
			::OutputDebugString(msgBuf);
#endif
			::CloseHandle(task->m_Handle);
		}
	}

}