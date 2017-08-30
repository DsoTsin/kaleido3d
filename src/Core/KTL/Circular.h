#pragma once 
#ifndef __k3d_Circular_h__
#define __k3d_Circular_h__

namespace k3d
{
    template <typename T, class TAlloc = kAllocator>
    class CircularBuffer
    {
    public:
        CircularBuffer(U32 Capacity)
        : m_Data (Capacity) 
        {
            m_Mask = Capacity - 1;
        }

        T& operator[](U32 Index)
        {
            return m_Data[Index & m_Mask];
        }

        const T& operator[](U32 Index) const
        {
            return m_Data[Index & m_Mask];
        }

        U32 NextIndex(U32 _CurIndex) const
        {
            return ((_CurIndex + 1) & m_Mask);
        }

        U32 PrevIndex(U32 _CurIndex) const
        {
            return ((_CurIndex - 1) & m_Mask);
        }

        U64 Capacity() const 
        {
            return m_Data.Capacity();
        }
    protected:
        DynArray<T, TAlloc> m_Data;
        U32 m_Mask;
    };

    // Thread Safe?
    template <typename T, class TAlloc = kAllocator>
    class CircularQueue : public CircularBuffer<T, TAlloc>
    {
        using Super = CircularBuffer<T, TAlloc>;
        using Super::m_Data;
    public:
        CircularQueue(U32 Capacity)
        : Super(Capacity)
        , m_Head(0)
        , m_Tail(0)
        , m_CurIndex(0)
        {}

        bool IsEmpty() const { return m_Head == m_Tail; }
        bool IsFull() const { return NextIndex(m_Tail) == m_Head; }
        void Empty() { m_Head == m_Tail; }

        bool Enqueue(const T& InElem)
        {
            auto _Tail = NextIndex(m_Tail);
            if (_Tail != m_Head)
            {
                m_Data[m_Tail] = InElem;
                __intrinsics__::AtomicCAS(&m_Tail, _Tail, m_Tail);
                return true;
            }
            return false;
        }

        bool EnQueue(T&& InElem)
        {
            auto _Tail = NextIndex(m_Tail);
            if (_Tail != m_Head)
            {
                m_Data[m_Tail] = InElem;
                __intrinsics__::AtomicCAS(&m_Tail, _Tail, m_Tail);
                return true;
            }
            return false;
        }

        bool DeQueue(T& OutElem)
        {
            if (m_Head != m_Tail)
            {
                OutElem = m_Data[m_Head];
                __intrinsics__::AtomicCAS(&m_Head, NextIndex(m_Head), m_Head);
                return true;
            }
            return false;
        }

        bool Peek(T& OutElem)
        {
            if (m_Head != m_Tail)
            {
                OutElem = m_Data[m_Head];
                return true;
            }
            return false;
        }
    private:
        U32 m_Head;
        U32 m_Tail;
        U32 m_CurIndex;
    };
}

#endif
