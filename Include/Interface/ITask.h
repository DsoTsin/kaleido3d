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
	virtual void    OnRun() = 0;

	virtual void	OnFinish() {}
	virtual void	OnStop() {}
	virtual void	OnResume() {}
	virtual void	OnCanceled() {}
};

enum class TaskPriority : uint32 {
	RealTime,
	BackGround,
	Normal
};

#endif
