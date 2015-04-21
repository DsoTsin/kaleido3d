#pragma once
#ifndef __TaskManager_h__
#define __TaskManager_h__
#include <Interface/IRunnable.h>
#include <KTL/Singleton.hpp>
#include <queue>
//#include <KTL/threadpool.h>

namespace k3d
{
  struct WThread
  {
    typedef unsigned long(STD_CALL *ThreadFunctionPtr)(void*);
    typedef void *  ThreadHandle;
    ThreadHandle  m_Handle;
    void  Join ()
    {
      Wait (this);
    }
    static void Wait (WThread * thread);
  };

  struct IBaseThread : public virtual IRunnable, public WThread
  {
    virtual void OnRun () override
    {
    }
  };

  enum class NamedThread : uint32
  {
    GamePlayThread = 0,
    AssetLoadingThread = 1,
    MainRenderingThread = 2,
    ShaderCompileThread = 3
  };

  class NameThreadTask;
/*
  class TaskManager : public Singleton<TaskManager>
  {
  public:
    static const int MaxNamedThreadNum = 4;
    TaskManager ();

    void Post (IBaseThread * task, TaskPriority priority = TaskPriority::Normal);
    void Post (IRunnable * task, NamedThread thread);
    void Suspend (IBaseThread * task);
    void Remove (IBaseThread * task);

    void RemoveAllTask ()
    {
      while (!m_ThreadQueue.empty ())
      {
        Remove (m_ThreadQueue.front ());
        m_ThreadQueue.pop ();
      }
    }

    void Init ()
    {
      initNamedThreadQueue ();
    }

    void ShutDown ()
    {
    }

    typedef std::queue<IRunnable *> TaskQueue;
    typedef std::queue<IBaseThread *> ThreadQueue;

  protected:

    void initNamedThreadQueue ();

  private:
    ThreadQueue m_ThreadQueue;
    TaskQueue m_NamedQueue[MaxNamedThreadNum];
    NameThreadTask *  m_NamedTask[MaxNamedThreadNum];
  };
*/
}

#endif
