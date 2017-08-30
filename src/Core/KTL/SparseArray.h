#pragma once

#ifndef __k3d_SparseArray_h__
#define __k3d_SparseArray_h__

#if 0
namespace k3d
{
    template <typename T>
    class SparseArray
    {
    public:

        SparseArray(int InitialCapacity = 10)
            : m_Keys(InitialCapacity)
            , m_Values(InitialCapacity)
        {
        }

        ~SparseArray()
        {
        }

        SparseArray & Put(int Key, T Value)
        {
            int Index = BinarySearch(m_Keys, Key);
            if (Index >= 0)
            {
                m_Values[Index] = Value;
            }
            else
            {
                Index = ~Index;
                if (Index < m_Keys.Count() /*&& m_Values[Index] == Deleted*/)
                {
                    m_Keys[Index] = Key;
                    m_Values[Index] = Value;
                    return;
                }

            }
            return *this;
        }

        //const T* Get(int Key) const
        //{
        //    int Index = BinarySearch(m_Keys, Key);
        //    if (Index >= 0))
        //    {
        //        return m_Values[Index];
        //    }
        //    else
        //    {
        //        return nullptr;
        //    }
        //}

        T Get(int Key, T Expected = T())
        {
            int Index = BinarySearch(m_Keys, Key);
            //if (Iter != m_Keys.end())
            {
                //return m_Values[*Iter];
                return Expected;
            }
            //else
            //{
            //    return Expected;
            //}
        }

        bool Remove(int Key)
        {
            return true;
        }

        //class KeyIterator
        //{
        //public:

        //private:
        //    KeyIterator();
        //};

        //class ValueIterator
        //{
        //public:

        //private:
        //    ValueIterator();
        //};

        //KeyIterator Keys() const;
        //ValueIterator Values() const;
        static int BinarySearch(DynArray<int> const& Array, int Value) 
        {
            int lo = 0;
            int hi = Array.Count() - 1;
            while (lo <= hi) 
            {
                int mid = (lo + hi) >> > 1;
                int midVal = Array[mid];
                if (midVal < Value) 
                {
                    lo = mid + 1;
                }
                else if (midVal > Value) 
                {
                    hi = mid - 1;
                }
                else 
                {
                    return mid;
                }
            }
            return ~lo;
        }
    private:
        DynArray<int> m_Keys;
        DynArray<T>   m_Values;
    };
}
#endif
#endif
