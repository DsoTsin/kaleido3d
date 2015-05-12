#pragma once 
#include <tuple>
#include <type_traits>
#include <memory>
#include <utility>
#include <thread>

#include "ThreadCommon.h"

namespace Concurrency {

	class ThreadInternal {
	public:
		ThreadInternal() : handle(nullptr)
		{}

		bool IsValid() { return handle != nullptr; }
#if defined(K3DPLATFORM_OS_WIN)
		void* handle;
#endif
	};

	class ThreadLauncherBase {
	public:
		ThreadLauncherBase();
		~ThreadLauncherBase()noexcept;

		void Launch( ThreadInternal * thr,  ThreadPriority priority );

		void Release();

		virtual void Go() = 0;

	private:

		static uint32 _STDCALL CallFunc(void *data);

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
			try {
			Run(this);
			}
			catch (...) {

			}
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