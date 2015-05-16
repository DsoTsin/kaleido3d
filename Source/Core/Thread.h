#pragma once 
#include "ThreadLauncher.h"
#include <thread>

namespace Concurrency {

	class Thread {
	public:
		enum Priority {
			Normal, 
			High
		};

		enum Status {
			Ready,
			Running,
			Finish
		};

		Thread(std::string const & name, Priority priority = Normal);
		Thread();
		virtual ~Thread();

		void SetPriority(Priority prio);


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

		Status GetThreadStatus();

	private:
		
		std::string m_ThreadName;
		Priority	m_ThreadPriority;
		Status		m_ThreadStatus;

	private:
		template<class FunctionUniPtr> inline
			void Launch(FunctionUniPtr&& _Tg)
		{	
			ThreadLauncher<FunctionUniPtr> launcher(std::forward<FunctionUniPtr>(_Tg));
			launcher.Launch(this);
		}
	};
}