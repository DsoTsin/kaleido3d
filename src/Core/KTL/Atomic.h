#pragma once

namespace k3d
{
    template <typename T>
    class Atomic
    {
        typedef Atomic<T> ThisType;
    public:

        static_assert(sizeof(T) <= sizeof(long), "");

        Atomic()
        {}

        Atomic(T const& Other) : m_Value(Other) {}

        void Store(T const& Val)
        {

        }

        void Load(T& Val)
        {

        }

        void Swap(ThisType const& Rhs)
        {
            __intrinsics__::AtomicCAS(&m_Value, Rhs.m_Value, 1);
        }

        T Get() const { return m_Value; }

    private:
        T m_Value;
    };

    class AtomicBool
    {
    public:
        AtomicBool(bool InVal = false) : m_Value(false) {}

        void Set(bool InVal)
        {
            m_Value = InVal;
        }
        bool Get() const
        {
            return m_Value;
        }

        operator bool() const
        {
            return m_Value;
        }

    private:
        bool m_Value;
    };
}