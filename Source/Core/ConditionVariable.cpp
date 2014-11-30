#include "Kaleido3D.h"
#include "ConditionVariable.h"
#include "LogUtil.h"
#if K3DPLATFORM_OS_WIN
#include "Windows/ConditionVariableImpl.h"
#endif

namespace k3d {

	ConditionVariable::ConditionVariable() : m_Handle(nullptr)
	{
		m_Handle = ConditionVariableImpl::createEvent(false, false, nullptr);
	}

	ConditionVariable::ConditionVariable(const char *name, bool autoRest, bool signaled) : m_Handle(nullptr)
	{
		m_Handle = ConditionVariableImpl::createEvent(autoRest, signaled, name);
	}

	void ConditionVariable::Release()
	{
		ConditionVariableImpl::closeEvent(m_Handle);
	}

	void ConditionVariable::Signal()
	{
		ConditionVariableImpl::setEvent(m_Handle);
	}

	void ConditionVariable::Reset()
	{
		ConditionVariableImpl::resetEvent(m_Handle);
	}
	
	WaitResult ConditionVariable::WaitFor(SpConditionVariable _event, uint32 millisecond)
	{
		return ConditionVariableImpl::waitSingleEvent(_event->m_Handle, millisecond);
	}

}