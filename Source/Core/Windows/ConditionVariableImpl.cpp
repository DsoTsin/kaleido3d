#include "Kaleido3D.h"
#include "ConditionVariableImpl.h"
#include "../LogUtil.h"

namespace k3d 
{

	namespace ConditionVariableImpl
	{

		ConditionVariable::Handle createEvent(bool autoReset, bool signaled, const char * eventName)
		{
			ConditionVariable::Handle handle = ::CreateEvent(NULL,
				autoReset ? FALSE : TRUE,
				signaled ? TRUE : FALSE,
				eventName);
			if (handle == NULL)
			{
				DBG_LINE_WITH_LAST_ERROR("ConditionVariableImpl","ConditionVariable::createEvent failed! ");
			}
			return handle;
		}

		void closeEvent(ConditionVariable::Handle & eventHandle)
		{
			::CloseHandle(eventHandle);
		}

		bool setEvent(ConditionVariable::Handle & eventHandle)
		{
			return (TRUE==::SetEvent(eventHandle));
		}

		bool resetEvent(ConditionVariable::Handle & eventHandle)
		{
			return TRUE == ::ResetEvent(eventHandle);
		}

		WaitResult waitSingleEvent(ConditionVariable::Handle eventHandle, uint32 ms)
		{
			WaitResult result = WaitResult::Success;
			if (eventHandle != nullptr) {
				DWORD dwWaitResult = ::WaitForSingleObject(eventHandle, ms);
				switch (dwWaitResult)
				{
				case WAIT_OBJECT_0:
					return result;
				case WAIT_FAILED:
					OutputDebugStringA("ConditionVariable::waitSingleEvent failed!!\n");
					result = WaitResult::Failed;
				case WAIT_ABANDONED:
					OutputDebugStringA("ConditionVariable::waitSingleEvent abandoned!!\n");
					result = WaitResult::Failed;
				case WAIT_TIMEOUT:
					OutputDebugStringA("ConditionVariable::waitSingleEvent timeout!!\n");
					result = WaitResult::TimeOut;
					break;
				default:
					break;
				}
				DBG_LINE_WITH_LAST_ERROR("ConditionVariableImpl","ConditionVariable::waitSingleEvent failed.");
			}
			return result;
		}
	}

}