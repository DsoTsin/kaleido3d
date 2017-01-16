#pragma once 
#include <string>
#include <mutex>
#include <atomic>
#include "../Os.h"

namespace Dispatch {

using String	= std::string;
using AtomicBool= std::atomic_bool;

class WorkItem;
class WorkGroup;

class K3D_API WorkQueue : public ::Os::Thread {
public:
	typedef WorkItem* PtrWorkItem;
	typedef WorkGroup* PtrWorkGroup;

	WorkQueue(String const & name, ::Os::ThreadPriority priority);

	WorkQueue & Queue(PtrWorkItem item);
	WorkQueue & Queue(PtrWorkGroup item);
	void		Loop();
	void		StopAll();
	bool		IsEmpty();
	PtrWorkItem	Front();
	void		Pop();
	void		Remove(PtrWorkItem item);

protected:

private:
	void		enqueue(PtrWorkItem item);

	WorkItem *	m_QueueHead;
	WorkItem *	m_CurItem;
	
	AtomicBool	m_Started;
	String		m_Name;
	::Os::Mutex		m_QueueLock;
	::Os::ConditionVariable m_QueueCV;
};

}