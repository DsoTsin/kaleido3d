#pragma once
#include <Config/Prerequisities.h>

namespace k3d {

	enum class WaitResult {
		TimeOut,
		Failed,
		Success
	};



	typedef std::shared_ptr<class ConditionVariable> SpConditionVariable;

	class ConditionVariable {
	public:

		ConditionVariable();
		explicit ConditionVariable(const char *eventName, bool autoRest = true, bool signaled = false);


#if K3DPLATFORM_OS_WIN
		typedef void *		Handle;
#endif

		void				Signal();
		void				Reset();
		void				Release();
		static WaitResult	WaitFor(SpConditionVariable _event, uint32 millisecond = 0xFFFFFFFF);

		ConditionVariable(const ConditionVariable &) = delete;
		ConditionVariable(const ConditionVariable &&) = delete;

	protected:

		Handle m_Handle;

	};


}