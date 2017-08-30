#pragma once
#include <CoreMinimal.h>
#include "Allocator.h"
#include "Tuple.h"
#include "DynArray.h"

namespace k3d {
namespace HashImpl {
    using k3d::U64;
    /**
     * Power Of Two Growth Policy
     */
    class P2Policy
    {
    public:

        static const U64 MIN_BUCKETS_SIZE = 2;

        P2Policy(U64& MinBucketCount)
        {
            MinBucketCount = MIN_BUCKETS_SIZE > MinBucketCount ?  MIN_BUCKETS_SIZE : MinBucketCount;
            MinBucketCount = RoundUpToPowerOf2(MinBucketCount);
            m_Mask = MinBucketCount - 1;
        }

        U64 BucketForHash(U64 Hash) const
        {
            return Hash & m_Mask;
        }

        U64 NextBucketCount() const 
        {
            if (m_Mask + 1 > MaxBucketCount() / 2)
            {

            }
            return (m_Mask + 1) << 1;
        }

        U64 MaxBucketCount() const
        {
            return 0xffffffff >> 1 + 1;
        }
    private:
        static constexpr bool IsPowerOf2(U64 value)
        {
            return value != 0 && (value & (value - 1)) == 0;
        }

        static U64 RoundUpToPowerOf2(U64 value) {
            if (value == 0) {
                return 1;
            }

            if (IsPowerOf2(value)) {
                return value;
            }

            --value;
            // 8 charbits
            for (U64 i = 1; i < sizeof(U64) * 8; i *= 2)
            {
                value |= value >> i;
            }

            return value + 1;
        }
        U64 m_Mask;
    };

    static const U64 NB_RESERVED_BITS_IN_NEIGHBORHOOD = 2;
    static const U64 SMALLEST_TYPE_MAX_BITS_SUPPORTED = 64;

    template<unsigned int MinBits, typename Enable = void>
    class FitBits
    {
    };

    template<unsigned int MinBits>
    class FitBits<MinBits, typename EnableIf<(MinBits > 0) && (MinBits <= 8)>::Type> 
    {
    public:
        using Type = uint8;
    };

    template<unsigned int MinBits>
    class FitBits<MinBits, typename EnableIf<(MinBits > 8) && (MinBits <= 16)>::Type> 
    {
    public:
        using Type = uint16;
    };

    template<unsigned int MinBits>
    class FitBits<MinBits, typename EnableIf<(MinBits > 16) && (MinBits <= 32)>::Type> 
    {
    public:
        using Type = uint32;
    };

    template<unsigned int MinBits>
    class FitBits<MinBits, typename EnableIf<(MinBits > 32) && (MinBits <= 64)>::Type> 
    {
    public:
        using Type = U64;
    };

    class BucketHash
    {
    public:
        BucketHash()
        {}
    private:
        U64 m_Hash;
    };

    template<
        class TValue,
        U64 NeighborhoodSize
    >
    class Bucket : public BucketHash
    {
        using StorageT = typename AlignedStorage<sizeof(TValue), alignof(TValue)>::Type;
        using Bitmap =
            typename FitBits<NeighborhoodSize + NB_RESERVED_BITS_IN_NEIGHBORHOOD>::Type;

    public:
        Bucket() K3D_NOEXCEPT : BucketHash(), m_NeighborBitmap(0) 
        {
            assert(Empty());
        }

        TValue& Value() K3D_NOEXCEPT { return *reinterpret_cast<TValue*>(&m_Storage); }
        const TValue& Value() const K3D_NOEXCEPT { return *reinterpret_cast<const TValue*>(&m_Storage); }

        bool HasOverflow() const { return (m_NeighborBitmap & 2) != 0; }
        bool Empty() const { return (m_NeighborBitmap & 1) == 0; }
        
        template<typename... Args>
        void AssignValue(U64 hash, Args&&... args) {
            assert(Empty());

            ::new (static_cast<void*>(&m_value)) TValue(Forward<Args>(args)...);
            SetEmpty(false);
            //this->SetHash(hash);
        }

        void Clear() 
        {
            if (!Empty())
            {
                DestroyValue();
            }
            m_Storage = 0;
        }

        void RemoveValue() K3D_NOEXCEPT
        {
            if (!Empty())
            {
                DestroyValue();
                SetEmpty(true);
            }
        }

        Bitmap NeighborInfo() const K3D_NOEXCEPT
        {
            return Bitmap(m_NeighborBitmap >> NB_RESERVED_BITS_IN_NEIGHBORHOOD);
        }

    private:
        void SetEmpty(bool isEmpty) K3D_NOEXCEPT
        {
            m_NeighborBitmap = isEmpty ? Bitmap(m_NeighborBitmap & ~1) : Bitmap(m_NeighborBitmap | 1);
        }
        void DestroyValue() K3D_NOEXCEPT
        {
            Value().~TValue();
        }

        Bitmap      m_NeighborBitmap;
        StorageT    m_Storage;
    };

    template <
        class TValue, 
        class THasher,
        class TGrowPolicy, 
        class TKeyEqual, 
        class TOverflowContainer, /*OverflowContainer will be used as containers for overflown elements. Usually it should be a list<ValueType>*/
        U64 NeighborhoodSize
    >
    class Hash : private THasher, private TKeyEqual, private TGrowPolicy
    {
        using TBucketContainer = DynArray< Bucket<TValue, NeighborhoodSize> >;
    public:

        Hash(U64 bucketCount,
            const THasher& hash,
            const TKeyEqual& equal,
            //const Allocator& alloc,
            float maxLoadFactor)
            : THasher(hash), TKeyEqual(equal), TGrowPolicy(bucketCount)
        {
            m_MaxLoadFactor = maxLoadFactor;
            m_Buckets.Resize(bucketCount + NeighborhoodSize - 1);
        }

        bool Empty() const noexcept 
        {
            return m_Count == 0;
        }

        void Clear() noexcept {
            for (auto& bucket : m_Buckets) {
                bucket.Clear();
            }

            m_OverflowContainer.Clear();
            m_Count = 0;
        }

        template<class K>
        U64 HashKey(const K& key) const
        {
            return THasher::operator()(key);
        }

        template<class K1, class K2>
        bool CompareKeys(const K1& key1, const K2& key2) const 
        {
            return TKeyEqual::operator()(key1, key2);
        }

        U64 BucketForHash(U64 hash) const 
        {
            return TGrowPolicy::BucketForHash(hash);
        }

        U64 FindEmptyBucket(U64 Offset) const 
        {
            const U64 limit = Min(Offset + MAX_PROBES_FOR_EMPTY_BUCKET, m_Buckets.Count());
            for (; Offset < limit; Offset++) {
                if (m_Buckets[Offset].Empty()) 
                {
                    return Offset;
                }
            }
            return m_Buckets.Count();
        }

    private:
        static const U64 MAX_PROBES_FOR_EMPTY_BUCKET = 12 * NeighborhoodSize;

        TBucketContainer    m_Buckets;
        TOverflowContainer  m_OverflowContainer;
        U64  m_Count;
        float   m_MaxLoadFactor;
        U64  m_LoadThreshold;
        U64  m_MinLoadFactorThreshold;
    };

}
}