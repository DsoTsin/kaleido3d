#pragma once

#include "../Event.h"
#include <Config/OSHeaders.h>

namespace k3d {

	namespace EventImpl
	{
		Event::EventHandle	createEvent(bool autoReset, bool signaled, const char * eventName);
		void				closeEvent(Event::EventHandle & eventHandle);
		bool				setEvent(Event::EventHandle & eventHandle);
		bool				resetEvent(Event::EventHandle & eventHandle);
		extern WaitResult	waitSingleEvent(Event::EventHandle eventHandle, uint32 ms);
	}

}