#include "ThreadImpl.h"

#include <strsafe.h>

namespace k3d
{
	namespace ThreadImpl
	{

		void createThread(IBaseThread & task, TaskPriority priority, WThread::ThreadFunctionPtr functionPtr)
		{
			createThread(task, priority, false, functionPtr);
		}

		void createThread(IBaseThread & task, TaskPriority priority, bool deferred, WThread::ThreadFunctionPtr functionPtr)
		{
			task.m_Handle = ::CreateThread(
				NULL,
				NULL,
				functionPtr,
				&task,
				deferred ? CREATE_SUSPENDED : 0, /* 0,CREATE_SUSPENDED,STACK_SIZE_PARAM_IS_A_RESERVATION */
				NULL);

			if (task.m_Handle != nullptr) {
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
					::SetThreadPriority(task.m_Handle, _tPriority);
#if _DEBUG
				if (res == TRUE) {
					OutputDebugStringA("::SetThreadPriority succeed.\n");
				}
#endif
			}
		}

		void resumeThread(IBaseThread & task)
		{
			::ResumeThread(task.m_Handle);
		}

		void suspendThread(IBaseThread & task)
		{
			::SuspendThread(task.m_Handle);
		}

		void terminateThread(IBaseThread & task)
		{
			if (task.m_Handle) {
				DWORD exCode = 0;
				::TerminateThread(task.m_Handle, exCode);
#if _DEBUG
				TCHAR msgBuf[1024] = { 0 };
				::StringCchPrintf(msgBuf, 1024, TEXT("exit code= %d\n"), exCode);
				::OutputDebugString(msgBuf);
#endif
				::CloseHandle(task.m_Handle);
			}
		}

		void sleep(uint32 milliSeconds) {
			::Sleep(milliSeconds);
		}
		
		void getSysInfo()
		{
			SYSTEM_INFO info;
			::GetSystemInfo(&info);

			TCHAR msgBuf[1024] = { 0 };
			::StringCchPrintf(msgBuf, 1024, TEXT("Avaliable CPU Core Num= %d\n"), info.dwNumberOfProcessors);
			::OutputDebugString(msgBuf);
		}
	}

}