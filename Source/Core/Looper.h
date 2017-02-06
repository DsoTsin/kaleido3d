#pragma once
#ifndef __Looper_h__
#define __Looper_h__
#include <queue>
#include <memory>
#include <thread>

#include "Os.h"

namespace k3d {

  using namespace Os;

  class Looper;

  class Handler
  {
  public:
    Handler ();
    virtual bool HandleMessage () = 0;
  };

	class Looper {
	public:
		Looper();


		virtual ~Looper();

		void StartLooper();

    static void Loop();


    typedef std::shared_ptr<Handler> spHandler;
    typedef std::shared_ptr<Thread> spThread;
  
  protected:
    static thread_local Looper sMyLooper;
    static thread_local std::queue<spHandler> mTaskQueue;

    static spThread sNamedThreads[4];

  private:
    bool mStopped;
	};
}

#endif