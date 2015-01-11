#pragma once
#include <Config/Prerequisities.h>

namespace k3d {

	enum class WaitResult {
		TimeOut,
		Failed,
		Success
	};

	class Event {
	public:

		Event();
		explicit Event(const char *eventName, bool autoRest = true, bool signaled = false);

		typedef std::shared_ptr<Event> SpEvent;

#if K3DPLATFORM_OS_WIN
		typedef void *		EventHandle;
#endif

		void				Signal();
		void				Reset();
		void				Release();
		static WaitResult	WaitFor(SpEvent _event, uint32 millisecond = 0xFFFFFFFF);

		Event(const Event &) = delete;
		Event(const Event &&) = delete;

	protected:

		EventHandle m_Handle;

	};

}