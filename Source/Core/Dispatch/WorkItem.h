#pragma once
#include <functional>

namespace Dispatch
{
	class WorkQueue;

	class WorkItem {
	public:
		WorkItem();
		virtual ~WorkItem();
		virtual void OnExec();
		void RemoveFromQueue();

		WorkQueue* GetOwningQueue();

	protected:
		friend class WorkQueue;

		WorkItem * m_Prev;
		WorkItem * m_Next;

		WorkQueue * m_OwningQueue;
	};

	template <class TFUN>
	class TWorkItem : public WorkItem {
	public:
		template <class U>
		TWorkItem(U && fun) {
			m_Fun = fun;
		}

		void OnExec() override {
			m_Fun();
		}

	private:
		TFUN m_Fun;
	};

	template <class BindFunction, class ...Args>
	WorkItem * Bind(BindFunction && bFun, Args ... args) {
		return new TWorkItem< std::function<void()> >( std::bind(bFun, args...) );
	}

}