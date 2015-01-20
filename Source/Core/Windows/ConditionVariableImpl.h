#pragma once

#include "../ConditionVariable.h"
#include <Config/OSHeaders.h>

namespace k3d {

	namespace ConditionVariableImpl
	{
		using CVHandle		= ConditionVariable::Handle;
		CVHandle			createEvent(bool autoReset, bool signaled, const char * eventName);
		void				closeEvent(CVHandle & eventHandle);
		bool				setEvent(CVHandle & eventHandle);
		bool				resetEvent(CVHandle & eventHandle);
		extern WaitResult	waitSingleEvent(CVHandle eventHandle, uint32 ms);
	}

}