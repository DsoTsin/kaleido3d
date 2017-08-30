#pragma once
#include "Hash.h"

namespace k3d
{
template <typename TKey, class TValue, typename THasher, typename TKeyEqualFunc, bool ThreadSafe = false, unsigned int NeighborhoodSize = 62>
class HashMap
{
    typedef HashMap<TKey, TValue, ThreadSafe> ThisType;
/*
    using overflow_container_type = std::list<std::pair<Key, T>, Allocator>;
    using Ht = HashImpl::Detail::Hash<
        Pair<TKey, TValue>,
        THasher, TKeyEqualFunc,
        Allocator, NeighborhoodSize,
        overflow_container_type
    >;*/
public:
    HashMap() K3D_NOEXCEPT {}
    ~HashMap() {}

    class Iterator
    {
    public:
        Iterator() {}
        ~Iterator() {}
        TKey Key() const {}
        TValue Value() const {}
        TValue& Value() {}
        void operator ++ ()
        {

        }
        operator bool() const
        {
            return false;
        }

        friend bool operator == (Iterator const& Lhs, Iterator const& Rhs)
        {
            return false;
        }
    };

    void        Insert(TKey const& Key, TValue const& Value) {}
    void        Remove(TKey const& Key) {}
    void        Remove(Iterator const& Iter) {}
    uint64      Count() const { return 0; }
    bool        Empty() const { return false; }
    void        Clear() {}
    Iterator    Find(TKey const& Key);

    Iterator    CreateIterator() const;
    Iterator&   CreateIterator();
    
    TValue      operator[] (TKey const& Key) const;
    TValue&     operator[] (TKey const& Key);

    ThisType&   operator=(ThisType const& Rhs);
    ThisType&   operator=(ThisType && Rhs);
private:
    Ht m_Ht;
};
}