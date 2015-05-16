#pragma once 
#include <tuple>
#include <type_traits>
#include <memory>
#include <utility>

namespace Concurrency {

	class Thread;

	struct ThreadLauncherBase {
		ThreadLauncherBase();
		~ThreadLauncherBase();

		void Launch(Thread * thr);

		void Release();

		virtual void Go() = 0;

	private:

		static uint32 _STDCALL CallFunc(void *data)
		{
			static_cast<ThreadLauncherBase *>(data)->Go();
			_Cnd_do_broadcast_at_thread_exit();
			return (0);
		}

		_Cnd_t m_Cond;
		_Mtx_t m_Mtx;
		bool m_Started;
	};


	template<class FunctionUniPtr>
	class ThreadLauncher : public ThreadLauncherBase
	{
	public:
		template<class _Other> 
		inline ThreadLauncher(_Other&& _Tgt)
			: m_Function(std::forward<_Other>(_Tgt))
		{
		}

		virtual void Go()
		{
			Run(this);
		}

	private:
		template<size_t... indexs>
		static void Execute(typename FunctionUniPtr::element_type& _Tup, std::integer_sequence<size_t, indexs...>)
		{
			// MSVC C++ XX? std::invoke
			std::invoke(std::move(std::get<indexs>(_Tup))...);
		}

		static void Run(ThreadLauncher *laucher) noexcept
		{	
			FunctionUniPtr local(std::forward<FunctionUniPtr>(laucher->m_Function));
			laucher->Release();
			Execute(
				*local,
				std::make_integer_sequence<size_t, std::tuple_size<typename FunctionUniPtr::element_type>::value> ()
				);
		}

		FunctionUniPtr m_Function;
	};

}