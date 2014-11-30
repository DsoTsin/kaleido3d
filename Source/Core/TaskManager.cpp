#include "Kaleido3D.h"
#include "TaskManager.h"

#if K3DPLATFORM_OS_WIN
#include "Windows/ConditionVariableImpl.h"
#include "Windows/ThreadImpl.h"
#elif K3DPLATFORM_OS_IOS
#include "iOS/ThreadImpl.h"
#endif

#include <functional>


namespace k3d {

	class NameThreadTask : public IBaseThread {
	public:
		using TaskQueue = TaskManager::TaskQueue;

		NameThreadTask(TaskQueue * queue)
			: m_ThreadQueue(nullptr)
		{
			m_ThreadQueue = queue;
		}

		void OnRun() override
		{
			assert(m_ThreadQueue != nullptr);
			while (true)
			{
				if (m_ThreadQueue->empty())
				{
					ThreadImpl::sleep(0);
				}
				else
				{
					IBaseThread * task = m_ThreadQueue->front();
					task->OnRun();
					m_ThreadQueue->pop();
				}
			}
		}

	private:
		TaskQueue	* m_ThreadQueue;
	};

	static void STD_CALL _createTask(IBaseThread * task) {
		if (task) {
			task->OnRun();
		}
	}

	void WThread::Wait(WThread * thread)
	{
		assert(thread);
#if K3DPLATFORM_OS_WIN
		ConditionVariableImpl::waitSingleEvent(thread->m_Handle, INFINITE);
#endif
	}

	TaskManager::TaskManager()
	{
		ThreadImpl::getSysInfo();
	}


	void TaskManager::initNamedThreadQueue()
	{
		for (int i = 0; i < MaxNamedThreadNum; ++i)
		{
			m_NamedTask[i] = new NameThreadTask(&m_NamedQueue[i]);
			ThreadImpl::createThread(*(m_NamedTask[i]), TaskPriority::Normal, true, (WThread::ThreadFunctionPtr)&_createTask);
			ThreadImpl::resumeThread(*(m_NamedTask[i]));
		}
	}

	void TaskManager::Post(IBaseThread * task, TaskPriority priority)
	{
		ThreadImpl::createThread(*task, priority, (WThread::ThreadFunctionPtr)&_createTask);
	}


	void TaskManager::Suspend(IBaseThread * task)
	{
		ThreadImpl::suspendThread(*task);
	}


	void TaskManager::Remove(IBaseThread * task)
	{
		ThreadImpl::terminateThread(*task);
	}

	void TaskManager::Post(IBaseThread * task, NamedThread thread)
	{
		m_NamedQueue[(uint32)thread].push(task);	
	}
	


}