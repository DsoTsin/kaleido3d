#pragma once 

#include <vector>

namespace Dispatch
{
	using std::vector;

	class WorkItem;
	class WorkQueue;

	class WorkGroup {
	public:
		WorkGroup() K3D_NOEXCEPT;
		~WorkGroup() K3D_NOEXCEPT;
		bool IsEmpty();
		WorkGroup& Add(WorkItem* item);

	private:
		friend class ::Dispatch::WorkQueue;
		vector<WorkItem*> m_ItemContainer;
	};
}
