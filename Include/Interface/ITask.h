#ifndef __ITask_H__
#define __ITask_H__

#include <Config/Prerequisities.h>

struct ITaskManager;
/*!
	\class ITask
*/
struct ITask
{
	virtual			~ITask() {}
	virtual void    Execute() = 0;

	virtual void	OnFinish() = 0;
	virtual void	OnStop() {}
	virtual void	OnResume() {}
	virtual void	OnCanceled() {}
};

enum class TaskPriority {
	RealTime,
	BackGround,
	Normal
};

struct ITaskManager
{
	virtual				  ~ITaskManager() {}
	virtual	void		AddTask( ITask * ) = 0;
	virtual void		LaunchTask( ITask * ) = 0;
  virtual size_t     GetRunningTasks() = 0;
};

#endif
