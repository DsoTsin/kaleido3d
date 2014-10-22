#ifndef __k3dTask__
#define __k3dTask__
#pragma once
#include <ITask.h>
#include <KTL/threadpool.h>

class k3dTaskManager;

class k3dTask : public ITask
{
public:
  k3dTask ();
  void    Execute ();
};


class k3dTaskManager : public ITaskManager
{
private:
  int     m_Capacity;
public:
  k3dTaskManager();
  k3dTaskManager( int capacity );
  ~k3dTaskManager();

  void    AddTask( ITask *task );
  void    LaunchTask( ITask * task );
  size_t  GetRunningTasks();

private:
  K3DDISCOPY( k3dTaskManager );
  std::thread_pool  *m_pThreadPool;
};

#endif
