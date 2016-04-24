#pragma once 
#include <string>
#include <mutex>
#include <atomic>
#include "../Thread/Thread.h"
#include "../Thread/ConditionVariable.h"
#include <Interface/IJob.h>

namespace Dispatch {

using String	= std::string;
using Mutex		= std::mutex;
using AtomicBool= std::atomic_bool;

class WorkItem;
class WorkGroup;

class K3D_API WorkQueue : public ::Concurrency::Thread {
public:
	typedef WorkItem* PtrWorkItem;
	typedef WorkGroup* PtrWorkGroup;

	WorkQueue(String const & name, ::Concurrency::ThreadPriority priority);

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
	::Concurrency::Mutex		m_QueueLock;
	::Concurrency::ConditionVariable m_QueueCV;
};

}