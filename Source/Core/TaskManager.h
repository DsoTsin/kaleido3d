#pragma once
#ifndef __TaskManager_h__
#define __TaskManager_h__
#include <Interface/ITask.h>
#include <KTL/Singleton.hpp>
#include <queue>
//#include <KTL/threadpool.h>

namespace k3d {

	struct WThread {
		typedef unsigned long(__stdcall *ThreadFunctionPtr)(void*);
		typedef void *				ThreadHandle;
		ThreadHandle				m_Handle;
		void						Join() { Wait(this); }
		static void					Wait(WThread * thread);
	};

	struct IBaseThread : public virtual ITask, public WThread {
		virtual void OnRun() override {}
	};

	enum class NamedThread : uint32 {
		GameThread = 0,
		AssetLoadingThread = 1,
		MainRenderingThread = 2
	};

	class NameThreadTask;

	class TaskManager : public Singleton<TaskManager> {
	public:

		static const int MaxNamedThreadNum = 3;

		TaskManager();
		
		void Post(IBaseThread * task, TaskPriority priority = TaskPriority::Normal);

		void Post(IBaseThread * task, NamedThread thread);

		void Suspend(IBaseThread * task);

		void Remove(IBaseThread * task);

		void RemoveAllTask()
		{
			while (!m_ThreadQueue.empty()) {
				Remove(m_ThreadQueue.front());
				m_ThreadQueue.pop();
			}
		}

		void Init() {
			initNamedThreadQueue();
		}

		void ShutDown() {}

		typedef std::queue<IBaseThread *> TaskQueue;

	protected:

		void initNamedThreadQueue();

	private:
		TaskQueue			m_ThreadQueue;
		TaskQueue			m_NamedQueue[MaxNamedThreadNum];
		NameThreadTask *	m_NamedTask[MaxNamedThreadNum];
	};

	//const int TaskManager::MaxNamedThreadNum = 3;
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
