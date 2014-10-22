#include "k3dTask.h"
#include <functional>

k3dTask::k3dTask()  {}

void k3dTask::Execute()
{
  std::cout << "K3D Task Excuted! " << std::this_thread::get_id();
}

k3dTaskManager::k3dTaskManager() :m_Capacity( 4 )
{ m_pThreadPool = new std::thread_pool( m_Capacity );  }

k3dTaskManager::k3dTaskManager( int capacity ) : m_Capacity( capacity ) 
{ m_pThreadPool = new std::thread_pool( m_Capacity ); }

k3dTaskManager::~k3dTaskManager() {
  delete m_pThreadPool;
}

void k3dTaskManager::AddTask( ITask *task )
{
  m_pThreadPool->enqueue( std::bind( &ITask::Execute, task ) );
}

size_t k3dTaskManager::GetRunningTasks()
{
  return m_pThreadPool->size();
}

void k3dTaskManager::LaunchTask( ITask * task )
{
  task->Execute();
}
