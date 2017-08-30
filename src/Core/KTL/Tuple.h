#pragma once
#ifndef __k3d_core_Tuple_h__
#define __k3d_core_Tuple_h__

namespace k3d
{
    template <typename T1, typename T2>
    struct Pair
    {
        Pair() : First(), Second() {}
        Pair(T1 const& _First, T2 const& _Second)
            : First(_First), Second(_Second)
        {}

        T1 First;
        T2 Second;
    };

    template<typename... TList> struct Tuple;
    template<> struct Tuple<> {};
    typedef Tuple<> NullTuple;

    template<typename T, typename... TList>
    struct Tuple<T, TList...> : public Tuple<TList...>
    {
      typedef T value_type;
      typedef Tuple<TList...> base_type;
      typedef Tuple<T,TList...> this_type;

      Tuple() : base_type(), Value() {}
      Tuple(const T & V, const TList&... Tails) : base_type(Tails...), Value(V) {}
      Tuple(const this_type & Other) : base_type(static_cast<const base_type&>(Other)), Value(Other.Value) {}

      const T & Head() const { return this->Value; }
    protected:
      T Value;
    };

    template<typename T>
    struct Tuple<T> : public NullTuple
    {
      typedef Tuple<T> this_type;
  
      Tuple() : Value() {}
      Tuple(const T & V) : Value(V) {}
      Tuple(const this_type & Other) : Value(Other.Value) {}

      const T & Head() const { return this->Value; }
      T & Head() { return this->Value; }
    protected:
      T Value;
    };

    template<unsigned int N, typename T, typename... TList>
    struct TupleAt;

    template<unsigned int N, typename T, typename... TList>
    struct TupleAt<N, Tuple<T, TList...> >
    {
      typedef typename TupleAt<N - 1, Tuple<TList...> >::value_type value_type;
      typedef typename TupleAt<N - 1, Tuple<TList...> >::tuple_type tuple_type;
    };

    template<typename T, typename... TList>
    struct TupleAt<0, Tuple<T, TList...> >
    {
      typedef T value_type;
      typedef Tuple<T, TList...> tuple_type;
    };

    template<>
    struct TupleAt<0, NullTuple>
    {
      typedef NullTuple value_type;
      typedef NullTuple tuple_type;
    };

    template<unsigned int N, typename... _TList>
    constexpr const typename TupleAt<N, Tuple<_TList...> >::value_type &
    TupleGet(const Tuple<_TList...> & _Tuple)
    {
      typedef Tuple<_TList...> tuple_type;
      typedef typename TupleAt<N, tuple_type>::tuple_type base_tuple_type;
      return static_cast<const base_tuple_type&>(_Tuple).Head();
    }

    /**
    template<unsigned int N, typename... TList>
    typename TupleAt<N, Tuple<TList...> >::value_type &
    TupleGet(Tuple<TList...> & Tuple)
    {
    typedef Tuple<TList...> tuple_type;
    typedef typename TupleAt<N, tuple_type>::tuple_type base_tuple_type;
    return static_cast<base_tuple_type&>(Tuple).Head();
    }
    **/

    template <typename... TList>
    Tuple<TList...> MakeTuple(TList... Args)
    {
      Tuple<TList...> NewTuple(Args...);
      return NewTuple;
    }
}

#endif
