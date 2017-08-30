#pragma
#ifndef __Functional_hpp__
#define __Functional_hpp__
#include "Tuple.hpp"
#include "TypeTrait.hpp"

K3D_COMMON_NS
{
template<typename>
class Function;

namespace Detail
{
    namespace FunctionImpl
    {
        struct FunctorStorageType
        {
            union
            {
                void*   First;
                char    Padding[2 * sizeof(void*)];
            };
        };

        struct FunctionStorage
        {
            FunctorStorageType Functor;
        };

        // Need Static Function Table

        template <class F, class A0, class... Args>
        KFORCE_INLINE auto Invoke_Impl(F&&f, A0&&a0, Args&&... args)
            ->decltype((Forward<A0>(a0).*f)(Forward<Args>(args)...))
        {
            return (Forward<A0>(a0).*f)(Forward<Args>(args)...);
        }

        template <class F, class A0, class>
        KFORCE_INLINE auto Invoke_Impl(F&&f, A0&&a0) -> decltype(Forward<A0>(a0).*f)
        {
            return Forward<A0>(a0).*f;
        }

        template <class F, class... Args>
        KFORCE_INLINE auto Invoke_Impl(F&&f, Args&&... args) -> decltype(Forward<F>(f)(Forward<Args>(args)...))
        {
            return Forward<F>(f)(Forward<Args>(args)...);
        }

        template <class T, class... Args>
        struct InvokeResult
        {
            typedef decltype(Invoke_Impl(Declval<T>(), Declval<Args>()...)) Type;
        };

        template <typename T> // a lambda captured functor is allowed
        class FnMemImpl
        {
        public:
            KFORCE_INLINE FnMemImpl(T func) : Func(func) {}

            // functor
            template <class... TArgs>
            typename InvokeResult<T, TArgs...>::Type operator()(TArgs&&... args) const
            {
                return Invoke_Impl(Func, Forward<TArgs>(args)...);
            }

        private:
            T Func;
        };
    }
}

template <class F, class... Args>
typename Detail::FunctionImpl::InvokeResult<F, Args...>::Type
Invoke(F&& f, Args&&... args)
{
    return Detail::FunctionImpl::Invoke_Impl(Forward<F>(f), Forward<Args>(args)...);
}

template <typename TRet, typename... TArgs>
class Function<TRet(TArgs...)>
{
public:
    Function() K3D_NOEXCEPT {}
    ~Function() K3D_NOEXCEPT 
    {
        // destroy function table
    }

    TRet operator()(TArgs... args) const
    {
        return RawCall(Storage.Functor, Forward<TArgs>(args)...);
    }

private:
    TRet (*RawCall)(const Detail::FunctionImpl::FunctorStorageType&, TArgs...);
    Detail::FunctionImpl::FunctionStorage Storage;
};


}
#endif