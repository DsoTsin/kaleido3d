#include "Kaleido3D.h"
#include "WorkItem.h"
#include "WorkQueue.h"

namespace Dispatch 
{
	WorkItem::WorkItem() K3D_NOEXCEPT
		: m_Prev(nullptr)
		, m_Next(nullptr)
		, m_OwningQueue(nullptr)
	{
	}

	WorkItem::~WorkItem()
	{
	}

	void WorkItem::OnExec()
	{
	}

	void WorkItem::RemoveFromQueue()
	{
		if (m_OwningQueue != nullptr) {
			m_OwningQueue->Remove(this);
		}
	}
	WorkQueue * WorkItem::GetOwningQueue()
	{
		return m_OwningQueue;
	}
}