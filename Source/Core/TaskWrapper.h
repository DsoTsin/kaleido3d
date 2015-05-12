#pragma once
#ifndef __TaskManager_h__
#define __TaskManager_h__
#include <Interface/IRunnable.h>
#include <KTL/Singleton.hpp>
#include <KTL/ThreadPool.hpp>

namespace Concurrency {

	using ThreadPool = std::thread_pool;
	using ThreadSinglePool = std::single_thread_pool;

    class TaskWrapper {
      public:
        
		static const int MaxNamedThreadNum = 4;
        
		TaskWrapper ();
		~TaskWrapper();

        void Init () {
			if(m_TouchThreadPool==nullptr)	
				m_TouchThreadPool = new ThreadPool(MaxNamedThreadNum);
        }

        void ShutDown () {
			if (m_TouchThreadPool != nullptr) {
				delete m_TouchThreadPool;
				m_TouchThreadPool = nullptr;
			}
        }

		template<class F, class... Args>
		auto Enqueue(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type> {
			return m_TouchThreadPool->enqueue(f, args...);
		}

      private:

		 static ThreadPool *m_TouchThreadPool;

    };

}

#endif
