#include "Kaleido3D.h"
#include "WorkQueue.h"
#include "WorkGroup.h"
#include "WorkItem.h"

namespace Dispatch {

WorkQueue::WorkQueue(k3d::String const& name, ::Os::ThreadPriority priority) K3D_NOEXCEPT
  : ::Os::Thread(
      [this]() {
        while (m_Started) {
          if (!IsEmpty()) {
            while (!IsEmpty()) {
              PtrWorkItem front = Front();
              if (front) {
                front->OnExec();
                Pop();
              }
            }
          } else {
            // Thread::Yield();
            m_QueueCV.Wait(&m_QueueLock);
          }
        }

      },
      name,
      priority)
  , m_QueueHead(new WorkItem)
  , m_CurItem(m_QueueHead)
{
}

WorkQueue&
WorkQueue::Queue(PtrWorkItem item)
{
#if defined(THREAD_SAFE)
  std::unique_lock<std::mutex> lock(m_QueueLock);
#endif
  enqueue(item);
  m_QueueCV.Notify();
  return *this;
}

WorkQueue&
WorkQueue::Queue(PtrWorkGroup item)
{
#if defined(THREAD_SAFE)
  std::unique_lock<std::mutex> lock(m_QueueLock);
#endif
  if (item != nullptr) {
    if (!item->m_ItemContainer.empty()) {
      for (PtrWorkItem& i : item->m_ItemContainer) {
        enqueue(i);
      }
      m_QueueCV.Notify();
    }
  }
  return (*this);
}

bool
WorkQueue::IsEmpty()
{
  return m_CurItem == m_QueueHead;
}

WorkQueue::PtrWorkItem
WorkQueue::Front()
{
  if (m_QueueHead->m_Next != nullptr) {
    return m_QueueHead->m_Next;
  }
  return nullptr;
}

void
WorkQueue::Pop()
{
#if defined(THREAD_SAFE)
  std::unique_lock<std::mutex> lock(m_QueueLock);
#endif
  PtrWorkItem next = m_QueueHead->m_Next;
  if (next != nullptr) {
    m_QueueHead->m_Next = next->m_Next;
    if (next->m_Next) {
      next->m_Next->m_Prev = m_QueueHead;
    } else {
      m_CurItem = m_QueueHead;
    }
  } else {
    m_QueueHead->m_Next = nullptr;
  }
#if defined(THREAD_SAFE)
#endif
}

void
WorkQueue::Remove(PtrWorkItem item)
{
#if defined(THREAD_SAFE)
  std::unique_lock<std::mutex> lock(m_QueueLock);
#endif
  if (item != nullptr) {
    if (item->m_Prev != nullptr) {
      PtrWorkItem prev = item->m_Prev;
      if (item->m_Next != nullptr) {
        prev->m_Next = item->m_Next;
        item->m_Next->m_Prev = prev;
      } else {
        prev->m_Next = nullptr;
      }
    }
  }
#if defined(THREAD_SAFE)
#endif
}

void
WorkQueue::enqueue(PtrWorkItem item)
{
  /*Initial Queue Head*/
  if (m_QueueHead->m_Next == nullptr) {
    m_QueueHead->m_Next = item;
    item->m_Prev = m_QueueHead;
    m_CurItem = m_QueueHead;
  }

  /*Link the item to the Queue*/
  if (m_CurItem != nullptr) {
    m_CurItem->m_Next = item;
    item->m_Prev = m_CurItem;
    m_CurItem = item;
  }

  item->m_OwningQueue = this;
}

void
WorkQueue::Loop()
{
  m_Started = true;
  Start();
}

void
WorkQueue::StopAll()
{
  m_Started = false;
}
}