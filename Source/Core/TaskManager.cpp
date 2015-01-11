#include "TaskManager.h"
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


	TaskManager::TaskManager()
	{
		initManager();
	}

	void TaskManager::Post(IBaseThread * task, TaskPriority priority)
	{
		createThread(task, priority);
	}

	void TaskManager::Suspend(IBaseThread * task)
	{
		suspendThread(task);
	}

	void TaskManager::Remove(IBaseThread * task)
	{
		terminateThread(task);
	}

	void TaskManager::RemoveAllTask()
	{
		while (!m_ThreadQueue.empty()) {
			Remove(m_ThreadQueue.front());
			m_ThreadQueue.pop();
		}
	}

}