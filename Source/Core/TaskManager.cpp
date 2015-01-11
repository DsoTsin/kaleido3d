#include "TaskManager.h"
#include "Windows/EventImpl.h"
#include "Windows/ThreadImpl.h"
#include <functional>


namespace k3d {
	/*
	TaskManager::TaskManager() :m_Capacity(4)
	{
		m_pThreadPool = new std::thread_pool(m_Capacity);
	}

	TaskManager::TaskManager(int capacity) : m_Capacity(capacity)
	{
		m_pThreadPool = new std::thread_pool(m_Capacity);
	}

	TaskManager::~TaskManager() {
		delete m_pThreadPool;
	}

	void TaskManager::AddTask(ITask *task)
	{
		m_pThreadPool->enqueue(std::bind(&ITask::Execute, task));
	}

	size_t TaskManager::GetRunningTasks()
	{
		return m_pThreadPool->size();
	}

	void TaskManager::LaunchTask(ITask * task)
	{
		task->Execute();
	}
	*/

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

	static void __stdcall _createTask(IBaseThread * task) {
		if (task) {
			task->OnRun();
		}
	}

	void WThread::Wait(WThread * thread)
	{
		assert(thread);
		EventImpl::waitSingleEvent(thread->m_Handle, INFINITE);
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