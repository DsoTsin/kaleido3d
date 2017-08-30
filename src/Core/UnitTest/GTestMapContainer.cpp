#include <CoreMinimal.h>
#include <KTL/Tuple.h>
#if K3DPLATFORM_OS_WINDOWS
#pragma comment(linker,"/subsystem:console")
#endif

static const size_t SMALLEST_TYPE_MAX_BITS_SUPPORTED = 64;
static const size_t NB_RESERVED_BITS_IN_NEIGHBORHOOD = 2;
template <unsigned int minBits, typename enable = void>
struct BitmapForBits
{};
template <unsigned int minBits>
struct BitmapForBits<minBits, typename std::enable_if<(minBits > 0) && (minBits <= 8)>::type> {
    using Type = k3d::U8;
};
template <unsigned int minBits>
struct BitmapForBits<minBits, typename std::enable_if<(minBits > 8) && (minBits <= 16)>::type> {
    using Type = k3d::U16;
};
template <unsigned int minBits>
struct BitmapForBits<minBits, typename std::enable_if<(minBits > 16) && (minBits <= 32)>::type> {
    using Type = k3d::U32;
};
template <unsigned int minBits>
struct BitmapForBits<minBits, typename std::enable_if<(minBits > 32) && (minBits <= 64)>::type> {
    using Type = k3d::U64;
};

template <bool bStoreHash>
struct HopscotchBucketHash
{
};
template <>
struct HopscotchBucketHash<true>
{
    using HashType = k3d::U32;
private:
    HashType m_Hash;
};

template <typename TValue, unsigned int NeighborSize, bool bStoreHash>
class HopscotchBucket : public HopscotchBucketHash<bStoreHash> {
private:
    static const size_t MIN_NEIGHBORHOOD_SIZE = 4;
    static const size_t MAX_NEIGHBORHOOD_SIZE = SMALLEST_TYPE_MAX_BITS_SUPPORTED - NB_RESERVED_BITS_IN_NEIGHBORHOOD;
    using BucketHash = HopscotchBucketHash<bStoreHash>;
public:
    using NeighborBitmap = typename BitmapForBits<NeighborSize + NB_RESERVED_BITS_IN_NEIGHBORHOOD>::Type;
private:
    using Storage = typename k3d::AlignedStorage<sizeof(TValue), alignof(TValue)>::Type;
    NeighborBitmap m_NeighborInfos; // 桶标记，bitmap存储邻近信息
    Storage m_Value;
private:
    void SetEmpty(bool bIsEmpty) {
        if (bIsEmpty) {
            m_NeighborInfos = NeighborBitmap(NeighborBitmap & ~1);
        } else {
            m_NeighborInfos = NeighborBitmap(NeighborBitmap | 1);
        }
    }
    void DestroyValue() K3D_NOEXCEPT {
        // deconstruct it
        Value().~TValue();
    }
public:
    TValue & Value() K3D_NOEXCEPT {
        //assert(!Empty());
        return *reinterpret_cast<TValue*>(&m_Value);
    }
    const TValue & Value() const K3D_NOEXCEPT {
        //assert(!Empty());
        return *reinterpret_cast<const TValue*>(&m_Value);
    }
    void SetOverflow(bool bHasOverflow) {
        if (bHasOverflow) {
            m_NeighborInfos = NeighborBitmap(NeighborBitmap | 2);
        } else {
            m_NeighborInfos = NeighborBitmap(NeighborBitmap & ~2);
        }
    }
    bool HasOverflow() const K3D_NOEXCEPT { return (m_NeighborInfos & 2) != 0; }
    bool Empty() const K3D_NOEXCEPT {return (m_NeighborInfos & 1) == 0; }
    void RemoveValue() K3D_NOEXCEPT {
        if (!Empty()) {
            DestroyValue();
            SetEmpty(true);
        }
    }
    void Clear() K3D_NOEXCEPT {
        if (!Empty()) {
            DestroyValue();
        }
        m_NeighborInfos = 0;
    }
    void SwapIntoEmptyBucket(HopscotchBucket& emptyBucket) {
        assert(emptyBucket.Empty());
    }
};
// <hash, <key, value>>
template <class TValue, // pair<k, v>  ,
    class KeySelect, // TValue.first   ,
    class ValueSelect, // TValue.second, has different ways to access, cache friendly (sparse map?)
    class Hasher, // operator () ()
    class KeyEqual,// ==
    class TAllocator, // allocate mem
    unsigned int NeighborSize, bool bStoreHash,
    class TGrowPolicy, // Prime? 2^?
    class TOverflowContainer> // List?
class HopscotchHash : private Hasher, private KeyEqual, private TGrowPolicy
{
public:
    using BucketType = HopscotchBucket<TValue, NeighborSize, bStoreHash>;
    using BucketContainer = k3d::DynArray<BucketType, TAllocator, 0>;
private:
    BucketContainer m_Buckets;
    size_t m_NumElements;
    float m_MaxLoadFactor;
    size_t m_LoadThreshold;
    size_t m_MinLoadFactorRehashThreshold;
public:

};

int main(int argc, const char* argv[])
{
    k3d::Tuple<int, k3d::U32> T_;
    k3d::TupleGet<0>(T_);
    k3d::TupleGet<1>(T_);
    return 0;
}