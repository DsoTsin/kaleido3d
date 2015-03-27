#pragma once
#ifndef __Looper_h__
#define __Looper_h__
#include <queue>
#include <memory>

namespace k3d {

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

    typedef std::shared_ptr<Handler> PtrHandler;
  
  private:
    std::queue<PtrHandler> mTaskQueue;
	};
}

#endif