/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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
        bool IsFull() const { return Super::NextIndex(m_Tail) == m_Head; }
        void Empty() { m_Head == m_Tail; }

        bool Enqueue(const T& InElem)
        {
            U32 _Tail = Super::NextIndex(m_Tail);
            if (_Tail != m_Head)
            {
                m_Data[m_Tail] = InElem;
                __intrinsics__::AtomicCAS((int*)&m_Tail, _Tail, m_Tail);
                return true;
            }
            return false;
        }

        bool EnQueue(T&& InElem)
        {
            auto _Tail = Super::NextIndex(m_Tail);
            if (_Tail != m_Head)
            {
                m_Data[m_Tail] = InElem;
                __intrinsics__::AtomicCAS((int*)&m_Tail, _Tail, m_Tail);
                return true;
            }
            return false;
        }

        bool DeQueue(T& OutElem)
        {
            if (m_Head != m_Tail)
            {
                OutElem = m_Data[m_Head];
                __intrinsics__::AtomicCAS((int*)&m_Head, Super::NextIndex(m_Head), m_Head);
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
