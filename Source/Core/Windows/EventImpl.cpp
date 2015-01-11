#include "EventImpl.h"
#include "../LogUtil.h"

namespace k3d 
{

	namespace EventImpl
	{

		Event::EventHandle createEvent(bool autoReset, bool signaled, const char * eventName)
		{
			Event::EventHandle handle = ::CreateEvent(NULL,
				autoReset ? FALSE : TRUE,
				signaled ? TRUE : FALSE,
				eventName);
			if (handle == NULL)
			{
				DBG_LINE_WITH_LAST_ERROR("Event::createEvent failed! ");
			}
			return handle;
		}

		void closeEvent(Event::EventHandle & eventHandle)
		{
			::CloseHandle(eventHandle);
		}

		bool setEvent(Event::EventHandle & eventHandle)
		{
			return (TRUE==::SetEvent(eventHandle));
		}

		bool resetEvent(Event::EventHandle & eventHandle)
		{
			return TRUE == ::ResetEvent(eventHandle);
		}

		WaitResult waitSingleEvent(Event::EventHandle eventHandle, uint32 ms)
		{
			WaitResult result = WaitResult::Success;
			if (eventHandle != nullptr) {
				DWORD dwWaitResult = ::WaitForSingleObject(eventHandle, ms);
				switch (dwWaitResult)
				{
				case WAIT_OBJECT_0:
					return result;
				case WAIT_FAILED:
					OutputDebugStringA("Event::waitSingleEvent failed!!\n");
					result = WaitResult::Failed;
				case WAIT_ABANDONED:
					OutputDebugStringA("Event::waitSingleEvent abandoned!!\n");
					result = WaitResult::Failed;
				case WAIT_TIMEOUT:
					OutputDebugStringA("Event::waitSingleEvent timeout!!\n");
					result = WaitResult::TimeOut;
					break;
				default:
					break;
				}
				DBG_LINE_WITH_LAST_ERROR("Event::waitSingleEvent failed.");
			}
			return result;
		}
	}

}