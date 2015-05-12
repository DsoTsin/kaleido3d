#pragma once 
#include "ThreadCommon.h"
#include "ThreadLauncher.h"
#include <thread>

namespace Concurrency {

	class Thread {
	public:
		Thread(std::string const & name, ThreadPriority priority = ThreadPriority::Normal);
		Thread();
		virtual ~Thread();

		void SetPriority(ThreadPriority prio);


		template<class TFunction,
		class... TArgs,
		class = typename std::enable_if
			<!std::is_same< typename std::decay<TFunction>::type, Thread>::value >::type>
		void Bind(TFunction &&fn, TArgs &&...args)
		{
			Launch(std::make_unique<
				std::tuple< std::decay<TFunction>::type, std::decay<TArgs>::type... > 
			>(std::forward<TFunction>(fn), std::forward<TArgs>(args)...));
		}

		void Start();
		void Join();
		void Terminate();

		ThreadStatus GetThreadStatus();

	private:
		
		std::string         m_ThreadName;
		ThreadPriority	    m_ThreadPriority;
		ThreadStatus		m_ThreadStatus;
		ThreadInternal *    m_ThreadInternal;

	private:
		template<class FunctionUniPtr> inline
			void Launch(FunctionUniPtr&& _Tg)
		{	
			ThreadLauncher<FunctionUniPtr> launcher(std::forward<FunctionUniPtr>(_Tg));
			launcher.Launch(m_ThreadInternal, m_ThreadPriority);
		}

	};
}