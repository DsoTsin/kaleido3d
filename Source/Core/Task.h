#ifndef __k3dTask__
#define __k3dTask__
#pragma once
#include <Interface/ITask.h>
#include <KTL/threadpool.h>

namespace k3d {

	class TaskManager : public ITaskManager
	{
	private:
		int     m_Capacity;
	public:
		TaskManager();
		TaskManager(int capacity);
		~TaskManager();

		void    AddTask(ITask *task);
		void    LaunchTask(ITask * task);
		size_t  GetRunningTasks();

	private:
		K3DDISCOPY(TaskManager);
		std::thread_pool  *m_pThreadPool;
	};
}

#endif
