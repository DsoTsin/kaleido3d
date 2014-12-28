#include "Task.h"
#include <functional>
namespace k3d {

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
}