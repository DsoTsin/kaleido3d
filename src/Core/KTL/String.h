#pragma once

#include "Archive.h"
#include <stdarg.h>

namespace k3d
{
template <typename BaseChar>
inline I64 CharLength(const BaseChar* cStr)
{
    if (cStr == nullptr)
        return 0;
    const BaseChar *eos = cStr;
    while (*eos++);
    return(eos - cStr - 1);
}

extern K3D_CORE_API int Vsnprintf(char*, int n, const char* fmt, va_list);

template <typename BaseChar, typename Allocator>
class StringBase
{
public:
    typedef I64							    CharPosition;
    typedef BaseChar*						CharPointer;
    typedef const BaseChar*					ConstCharPointer;
    typedef StringBase<BaseChar, Allocator> ThisString;

    enum CaseOption
    {
        CaseSensitive,
        CaseIgnore
    };

    static const CharPosition npos = (CharPosition)-1;

    StringBase() K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(0)
    {
    }


    explicit StringBase(I64 preAllocSize, bool bAssignLength = false) K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(preAllocSize)
    {
        m_pStringData = Allocate(m_Capacity);
        if (bAssignLength)
        {
            m_StringLength = preAllocSize - 1; // Real String Length
        }
    }

    StringBase(I64 desiredSize, BaseChar holderChar) K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(desiredSize)
        , m_Capacity(desiredSize)
    {
        m_pStringData = Allocate(m_Capacity);
        for (I64 i = 0; i < m_Capacity; i++)
        {
            m_pStringData[i] = holderChar;
        }
    }

    StringBase(const void * pData, size_t szData) K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(0)
    {
        I64 calLength = szData / sizeof(BaseChar);
        I64 remain = szData % sizeof(BaseChar);
        if (remain == 0)
        {
            m_StringLength = calLength;
            m_Capacity = m_StringLength + 1;
            m_pStringData = Allocate(m_Capacity);
            memcpy(m_pStringData, pData, szData);
            m_pStringData[m_StringLength] = 0;
        }
    }

    StringBase(ConstCharPointer pStr) K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(0)
    {
        m_StringLength = CharLength(pStr);
        if (m_StringLength)
        {
            m_Capacity = (I64)(1.5f * m_StringLength + 0.5f);
            m_pStringData = Allocate(m_Capacity);
            memcpy(m_pStringData, pStr, m_StringLength * sizeof(BaseChar));
            m_pStringData[m_StringLength] = 0;
        }
    }

    StringBase(const ThisString & rhs) K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(rhs.m_Capacity)
        , m_StringAllocator(rhs.m_StringAllocator)
    {
        Assign(rhs);
    }

    StringBase(ThisString && rhs) K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
    {
        MoveAssign(Move(rhs));
    }

    ~StringBase() K3D_NOEXCEPT
    {
        if (m_pStringData)
        {
            Deallocate();
        }
        m_pStringData = nullptr;
        m_StringLength = 0;
    }

    bool                Empty() const { return m_StringLength == 0; }
    I64				    Length() const { return m_StringLength; }
    CharPointer	        Data() { return m_pStringData; }
    ConstCharPointer	Data() const { return m_pStringData; }
    ConstCharPointer	CStr() const { return m_pStringData; }
    ConstCharPointer    operator*() const { return m_pStringData; }

    ThisString&			operator=(const ThisString& rhs) { Assign(rhs); return *this; }
    ThisString&			operator=(ThisString&& rhs) { MoveAssign(Move(rhs)); return *this; }
    ThisString&         operator+=(const ThisString& rhs);
    ThisString&         operator+=(const BaseChar& rhs);
    BaseChar			operator[](U64 id) const;
    BaseChar&			operator[](U64 id);
    ThisString&         AppendSprintf(const BaseChar* fmt, ...);
    void				Swap(ThisString& rhs);

    void				Resize(CharPosition newSize);
    CharPosition	    FindFirstOf(const BaseChar* Str) const;
    CharPosition        FindFirstNotOf(const BaseChar* Str) const;
    CharPosition        FindLastOf(const BaseChar* Str) const;
    CharPosition        FindLastNotOf(BaseChar _BaseChar) const;
    CharPosition        FindLastNotOf(ThisString const& _Str) const;
    CharPosition        Find(ConstCharPointer _Str, CharPosition StartPos = 0, CaseOption Opt = CaseSensitive) const;
    ThisString          SubStr(CharPosition _Start, CharPosition _Length) const;
    void                Erase(CharPosition _Position, CharPosition _Count = 1);
    ThisString&         ReCalculate();

    ThisString          ToUpper() const;
    ThisString          ToLower() const;

    static ThisString   Format(const BaseChar* fmt, ...);

    template <typename T, typename A>
    friend Archive&     operator<<(Archive & ar, StringBase<T,A> const& str);

    template <typename T, typename A>
    friend Archive&     operator >> (Archive & ar, StringBase<T,A> & str);

protected:
    CharPointer			Allocate(U64 stringLength);
    void				Deallocate();

    void				MoveAssign(ThisString && rhs);
    void				Assign(ThisString const& rhs);

private:
    CharPointer			m_pStringData;
    I64				    m_StringLength;
    I64                 m_Capacity;
    Allocator			m_StringAllocator;
};

template <typename BaseChar, typename Allocator>
KFORCE_INLINE BaseChar* StringBase<BaseChar, Allocator>::Allocate(U64 length)
{
    return reinterpret_cast<BaseChar*>(m_StringAllocator.allocate(sizeof(BaseChar)*length, 0));
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::Deallocate()
{
    m_StringAllocator.deallocate(m_pStringData, sizeof(BaseChar)*m_StringLength);
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::MoveAssign(StringBase<BaseChar, Allocator> && rhs)
{
    m_pStringData = rhs.m_pStringData;
    m_StringLength = rhs.m_StringLength;
    m_Capacity = rhs.m_Capacity;
    m_StringAllocator = Move(rhs.m_StringAllocator);
    rhs.m_pStringData = nullptr;
    rhs.m_StringLength = 0;
    rhs.m_Capacity = 0;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::Assign(StringBase<BaseChar, Allocator> const & rhs)
{
    if (m_pStringData != rhs.m_pStringData)
    {
        if (m_pStringData) 
        {
            if (rhs.Length() > m_Capacity - 1)
            {
                Deallocate();
                m_Capacity = rhs.m_StringLength + 1;
                m_pStringData = Allocate(m_Capacity);
            }
        }
        else
        {
            m_Capacity = rhs.m_StringLength + 1;
            m_pStringData = Allocate(m_Capacity);
        }
        m_StringLength = rhs.m_StringLength;
        memcpy(m_pStringData, rhs.m_pStringData, rhs.m_StringLength * sizeof(BaseChar));
        m_pStringData[m_StringLength] = 0;
    }
}

template <typename BaseChar, typename Allocator>
StringBase<BaseChar, Allocator>&
StringBase<BaseChar, Allocator>::AppendSprintf(const BaseChar *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int length = Vsnprintf(m_pStringData + m_StringLength, int(m_Capacity - m_StringLength), fmt, va);
    va_end(va);

    auto newLen = length + m_StringLength;
    if (newLen >= m_Capacity)
    {
        m_Capacity = (I64)(1.33f * newLen + 1.0f);
        BaseChar* newString = Allocate(m_Capacity);
        memcpy(newString, m_pStringData, m_StringLength * sizeof(BaseChar));

        va_list newVa;
        va_start(newVa, fmt);
        Vsnprintf(newString + m_StringLength, int(m_Capacity - m_StringLength), fmt, newVa);
        va_end(newVa);

        Deallocate();
        m_pStringData = newString;
    }

    m_StringLength = newLen;

    return *this;
}


template <typename BaseChar, typename Allocator>
StringBase<BaseChar, Allocator>
StringBase<BaseChar, Allocator>::Format(const BaseChar *fmt, ...)
{
    static BaseChar Buffer[16] = { 0 };
    va_list va;
    va_start(va, fmt);
    int PreAllocLength = Vsnprintf(Buffer, 16, fmt, va);
    va_end(va);

    StringBase<BaseChar, Allocator> FormatedString(PreAllocLength + 1, true);
    va_list newVa;
    va_start(newVa, fmt);
    Vsnprintf(FormatedString.Data(), PreAllocLength + 1, fmt, newVa);
    va_end(newVa);
    return FormatedString;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::Swap(StringBase<BaseChar, Allocator> & rhs)
{
    BaseChar* p = rhs.m_pStringData;
    rhs.m_pStringData = m_pStringData;
    m_pStringData = p;

    U64 l = rhs.m_StringLength;
    rhs.m_StringLength = m_StringLength;
    m_StringLength = l;

    U64 c = rhs.m_Capacity;
    rhs.m_Capacity = m_Capacity;
    m_Capacity = c;

    Allocator a = rhs.m_StringAllocator;
    rhs.m_StringAllocator = m_StringAllocator;
    m_StringAllocator = a;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::Resize(CharPosition newSize)
{
    auto newCapacity = (I64)(1.1f * newSize + 1.0f);
    if (newCapacity > m_Capacity)
    {
        m_Capacity = newCapacity;
        auto pStringData = Allocate(m_Capacity);
        if (m_StringLength)
        {
            memcpy(pStringData, m_pStringData, sizeof(BaseChar) * m_StringLength);
            pStringData[m_StringLength] = 0;
            Deallocate();
        }
        else
        {
            memset(pStringData, 0, sizeof(BaseChar) * m_Capacity);
        }
        m_pStringData = pStringData;
    }
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE BaseChar StringBase<BaseChar, Allocator>::operator[](U64 id) const
{
    return m_pStringData[id];
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE BaseChar& StringBase<BaseChar, Allocator>::operator[](U64 id)
{
    return m_pStringData[id];
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>&
StringBase<BaseChar, Allocator>::operator+=(StringBase<BaseChar, Allocator> const& rhs)
{
    auto rLen = rhs.m_StringLength;
    auto newLen = m_StringLength + rLen;
    if (newLen >= m_Capacity)
    {
        m_Capacity = U64(1.5*newLen + 1);
        auto pNewData = Allocate(m_Capacity);
        if (m_pStringData)
        {
            memcpy(pNewData, m_pStringData, m_StringLength * sizeof(BaseChar));
            Deallocate();
        }
        m_pStringData = pNewData;
    }
    memcpy(m_pStringData + m_StringLength, rhs.m_pStringData, rLen * sizeof(BaseChar));
    m_pStringData[newLen] = 0;
    m_StringLength = newLen;
    return *this;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>&
StringBase<BaseChar, Allocator>::operator+=(BaseChar const& rhs)
{
    auto newLen = m_StringLength + 1;
    if (newLen >= m_Capacity)
    {
        m_Capacity = I64(1.5*newLen + 1);
        auto pNewData = Allocate(m_Capacity);
        if (m_pStringData)
        {
            memcpy(pNewData, m_pStringData, m_StringLength * sizeof(BaseChar));
            Deallocate();
        }
        m_pStringData = pNewData;
    }
    m_pStringData[m_StringLength] = rhs;
    m_pStringData[newLen] = 0;
    m_StringLength = newLen;
    return *this;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>
operator+(StringBase<BaseChar, Allocator> const& lhs, StringBase<BaseChar, Allocator> const& rhs)
{
    StringBase<BaseChar, Allocator> ret(lhs);
    ret += rhs;
    return ret;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>
operator+(StringBase<BaseChar, Allocator> const& lhs, BaseChar const* rhs)
{
    StringBase<BaseChar, Allocator> Ret(lhs.Length() + CharLength(rhs) + 1);
    Ret += lhs;
    Ret += rhs;
    return Ret;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>
operator+(BaseChar const* lhs, StringBase<BaseChar, Allocator> const& rhs)
{
    StringBase<BaseChar, Allocator> Ret(rhs.Length() + CharLength(lhs) + 1);
    Ret += lhs;
    Ret += rhs;
    return Ret;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE bool operator==(StringBase<BaseChar, Allocator> const& lhs, StringBase<BaseChar, Allocator> const& rhs) K3D_NOEXCEPT
{
    if (&lhs == &rhs)
        return true;
    return ((lhs.Length() == rhs.Length())
        && (memcmp(lhs.Data(), rhs.Data(), lhs.Length() * sizeof(BaseChar)) == 0));
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::CharPosition
  StringBase<BaseChar, Allocator>::FindFirstOf(const BaseChar* Str) const
{
  auto Len = CharLength(Str);
  StringBase<BaseChar, Allocator>::CharPosition p = 0;
  while (p < m_StringLength)
  {
    for (I64 i = 0; i < Len; i++)
    {
      if (m_pStringData[p] == Str[i])
        return p;
    }
    ++p;
  }
  return StringBase<BaseChar, Allocator>::npos;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::CharPosition
StringBase<BaseChar, Allocator>::FindFirstNotOf(const BaseChar* Str) const
{
  auto Len = CharLength(Str);
  CharPosition p = 0;
  while (p < m_StringLength)
  {
    for (I64 i = 0; i < Len; i++)
    {
      if (m_pStringData[p] != Str[i])
        return p;
    }
    ++p;
  }
  return StringBase<BaseChar, Allocator>::npos;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::CharPosition
StringBase<BaseChar, Allocator>::FindLastNotOf(BaseChar _BaseChar) const
{
  CharPosition p = m_StringLength - 1;
  while (p != StringBase<BaseChar, Allocator>::npos
    && p >= 0)
  {
    if (m_pStringData[p] != _BaseChar)
      return p;
    --p;
  }
  return StringBase<BaseChar, Allocator>::npos;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::CharPosition
StringBase<BaseChar, Allocator>::FindLastNotOf(ThisString const& _Str) const
{
    CharPosition p = m_StringLength - 1;
    while (p != StringBase<BaseChar, Allocator>::npos
        && p >= 0)
    {
        if (m_pStringData[p] != _Str[0])
            return p;
        --p;
    }
    return StringBase<BaseChar, Allocator>::npos;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::CharPosition
StringBase<BaseChar, Allocator>::FindLastOf(const BaseChar* Str) const
{
    CharPosition p = m_StringLength - 1;
    while (p != StringBase<BaseChar, Allocator>::npos
        && p >= 0)
    {
        auto Len = CharLength(Str);
        for (I64 i = 0; i < Len; i++)
        {
            if (m_pStringData[p] == Str[i])
                return p;
        }
        --p;
    }
    return StringBase<BaseChar, Allocator>::npos;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::Erase(
    typename StringBase<BaseChar, Allocator>::CharPosition _StartPosition,
    typename StringBase<BaseChar, Allocator>::CharPosition _Count)
{
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>&
StringBase<BaseChar, Allocator>::ReCalculate()
{
    m_StringLength = CharLength(m_pStringData);
    return *this;
}

/**
 * KMP String searching
 */
template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::CharPosition
StringBase<BaseChar, Allocator>::Find(ConstCharPointer _Str, CharPosition StartPos, CaseOption Opt) const
{
  if (m_pStringData == nullptr || _Str == nullptr || m_pStringData[0] == 0 || _Str[0] == 0)
    return ThisString::npos;
  auto len = CharLength(_Str);
  if (len > m_StringLength) return ThisString::npos;
  DynArray<CharPosition> match(len, -1);
  CharPosition j = ThisString::npos;
  for (CharPosition i = 1; i < (CharPosition)len; i++)
  {
    while (j > 0 && _Str[i] != _Str[j + 1]) j = match[j];
    if (_Str[i] == _Str[j + 1]) j++;
    match[i] = j;
  }
  j = ThisString::npos;
  for (CharPosition i = 0; i < (CharPosition)m_StringLength; i++)
  {
    while (j >= 0 && m_pStringData[i] != _Str[j + 1]) j = match[j];
    if (m_pStringData[i] == _Str[j + 1]) j++;
    if (j == (CharPosition)(len - 1)) return i - len + 1;
  }
  return ThisString::npos;
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::ThisString
StringBase<BaseChar, Allocator>::SubStr(
    typename StringBase<BaseChar, Allocator>::CharPosition _Start,
    typename StringBase<BaseChar, Allocator>::CharPosition _Length) const
{
    return ThisString(m_pStringData + _Start, _Length);
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::ThisString
StringBase<BaseChar, Allocator>::ToUpper() const
{
  return ThisString();
}

template <typename BaseChar, typename Allocator>
KFORCE_INLINE typename StringBase<BaseChar, Allocator>::ThisString
StringBase<BaseChar, Allocator>::ToLower() const
{
  return ThisString();
}

template <typename BaseChar, typename Allocator>
Archive& operator<<(Archive & ar, StringBase<BaseChar, Allocator> const& str)
{
    ar << str.m_Capacity << str.m_StringLength;
    ar.ArrayIn(str.CStr(), str.Length());
    return ar;
}

template <typename BaseChar, typename Allocator>
Archive& operator >> (Archive & ar, StringBase<BaseChar, Allocator> & str)
{
    ar >> str.m_Capacity >> str.m_StringLength;
    str.m_pStringData = str.Allocate(str.m_Capacity);
    ar.ArrayOut(str.m_pStringData, str.Length());
    str.m_pStringData[str.m_StringLength] = 0;
    return ar;
}


typedef StringBase<char, kAllocator> String;

extern K3D_CORE_API String Base64Encode(String const & in);
extern K3D_CORE_API String Base64Decode(String const& in);
extern K3D_CORE_API String MD5Encode(String const& in);
}

#include <string>

namespace std
{
    template <int size>
    struct _FNVHash
    {
        size_t operator()(const unsigned char *pData, size_t count);
    };

    template <>
    struct _FNVHash<8>
    {
        inline size_t operator()(const unsigned char *pData, size_t count)
        {
            const size_t fnvOffsetBase = 14695981039346656037ULL;
            const size_t fnvPrime = 1099511628211ULL;
            size_t val = fnvOffsetBase;
            for (size_t i = 0; i < count; ++i)
            {   // fold in another byte
                val ^= (size_t)pData[i];
                val *= fnvPrime;
            }
            return (val);
        }
    };

    template <>
    struct _FNVHash<4>
    {
        inline size_t operator()(const unsigned char *pData, size_t count)
        {
            const size_t fnvOffsetBase = 2166136261U;
            const size_t fnvPrime = 16777619U;
            size_t val = fnvOffsetBase;
            for (size_t i = 0; i < count; ++i)
            {
                val ^= (size_t)pData[i];
                val *= fnvPrime;
            }
            return (val);
        }
    };

    template<>
    struct hash<k3d::String>
    {
        size_t operator()(const k3d::String& val) const
        {
            _FNVHash<sizeof(size_t)> _Hasher;
            return _Hasher((const unsigned char *)val.CStr(), val.Length());
        }
    };
}
