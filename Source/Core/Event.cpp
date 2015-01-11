#include "Event.h"
#include "LogUtil.h"

#include "Windows/EventImpl.h"

namespace k3d {

	Event::Event() : m_Handle(nullptr)
	{
		m_Handle = EventImpl::createEvent(false, false, nullptr);
	}

	Event::Event(const char *name, bool autoRest, bool signaled) : m_Handle(nullptr)
	{
		m_Handle = EventImpl::createEvent(autoRest, signaled, name);
	}

	void Event::Release()
	{
		EventImpl::closeEvent(m_Handle);
	}

	void Event::Signal()
	{
		EventImpl::setEvent(m_Handle);
	}

	void Event::Reset()
	{
		EventImpl::resetEvent(m_Handle);
	}
	
	WaitResult Event::WaitFor(SpEvent _event, uint32 millisecond)
	{
		return EventImpl::waitSingleEvent(_event->m_Handle, millisecond);
	}

}