#include "Kaleido3D.h"
#include "WorkGroup.h"
#include "WorkItem.h"

namespace Dispatch
{
	WorkGroup::WorkGroup() K3D_NOEXCEPT
	{
	}
	WorkGroup::~WorkGroup()
	{
	}
	bool WorkGroup::IsEmpty()
	{
		return m_ItemContainer.empty();
	}
	WorkGroup & WorkGroup::Add(WorkItem * item)
	{
		m_ItemContainer.push_back(item);
		return *this;
	}
}