#include "Kaleido3D.h"

#include "Looper.h"
#include <Config/OSHeaders.h>

namespace k3d
{

  Looper::Looper ()
    : mStopped (true)
  {
  }

  Looper::Looper (NamedThread Thr)
    : mStopped (true)
  {
    if (sNamedThreads[(uint32)Thr]==nullptr)
    {
      mStopped = false;
      sNamedThreads[(uint32)Thr] = std::shared_ptr<std::thread> (
        new std::thread ([this]()
      {
        while (!mStopped)
        {
          while (!mTaskQueue.empty ())
          {
            spHandler handle = mTaskQueue.front ();
            handle->HandleMessage ();
            mTaskQueue.pop ();
          }
          ::Sleep (0);
        }
      }
      ));
    }
  }

  Looper::~Looper ()
  {
  }

  void Looper::StartLooper ()
  {
  }

  void Looper::Loop ()
  {
  }
}