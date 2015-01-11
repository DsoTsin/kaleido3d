#pragma once
#ifndef __TaskManager_h__
#define __TaskManager_h__
#include <Interface/ITask.h>
#include <KTL/Singleton.h>
#include <queue>
//#include <KTL/threadpool.h>

namespace k3d {

	struct WThread {
		typedef void * ThreadHandle;
		ThreadHandle m_Handle;
	};

	struct IBaseThread : public virtual ITask, public WThread {
		virtual void OnRun() override {}
	};


	class TaskManager : public Singleton<TaskManager> {
	public:

		TaskManager();

		void Post(IBaseThread * task, TaskPriority priority = TaskPriority::Normal);

		void Suspend(IBaseThread * task);

		void Remove(IBaseThread * task);

		void RemoveAllTask();

	protected:

		void initManager();

		void createThread(IBaseThread * task, TaskPriority priority);
		void resumeThread(IBaseThread * task);
		void suspendThread(IBaseThread * task);
		void terminateThread(IBaseThread * task);

	private:

		typedef std::queue<IBaseThread *> TaskQueue;

		TaskQueue	m_ThreadQueue;
	};

	/*
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
	*/
}

#endif
