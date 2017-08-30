#pragma once
#include "../Os.h"
#include <KTL/String.hpp>
//#include <atomic>
//#include <mutex>

namespace Dispatch {

using String = k3d::String;
//using AtomicBool = std::atomic_bool;

class WorkItem;
class WorkGroup;

class K3D_CORE_API WorkQueue : public ::Os::Thread
{
public:
  typedef WorkItem* PtrWorkItem;
  typedef WorkGroup* PtrWorkGroup;

  WorkQueue(String const& name, ::Os::ThreadPriority priority) K3D_NOEXCEPT;

  WorkQueue& Queue(PtrWorkItem item);
  WorkQueue& Queue(PtrWorkGroup item);
  void Loop();
  void StopAll();
  bool IsEmpty();
  PtrWorkItem Front();
  void Pop();
  void Remove(PtrWorkItem item);

protected:
private:
  void enqueue(PtrWorkItem item);

  WorkItem* m_QueueHead;
  WorkItem* m_CurItem;

  bool m_Started;
  String m_Name;
  ::Os::Mutex m_QueueLock;
  ::Os::ConditionVariable m_QueueCV;
};
}