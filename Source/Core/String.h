#pragma once
#include <KTL/Allocator.hpp>
#include "Archive.h"

K3D_COMMON_NS
{
template <typename BaseChar>
inline uint64 CharLength(const BaseChar* cStr)
{
	const BaseChar *eos = cStr;
	while (*eos++);
	return(eos - cStr - 1);
}
    
extern K3D_API int Vsnprintf(char*, int n, const char* fmt, va_list);

template <typename BaseChar, typename Allocator>
class StringBase
{
public:
	typedef BaseChar*						CharPointer;
	typedef const BaseChar*					ConstCharPointer;
	typedef StringBase<BaseChar, Allocator> ThisString;
	typedef uint64							CharPosition;

    StringBase() K3D_NOEXCEPT
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(0)
    {
    }
    
    StringBase(const void * pData, size_t szData)
        : m_pStringData(nullptr)
        , m_StringLength(0)
        , m_Capacity(0)
    {
        uint64 calLength = szData/sizeof(BaseChar);
        uint64 remain = szData%sizeof(BaseChar);
        if(remain == 0)
        {
            m_StringLength = calLength;
            m_Capacity = m_StringLength + 1;
            m_pStringData = Allocate(m_Capacity);
            memcpy(m_pStringData, pData, szData);
            m_pStringData[m_StringLength] = 0;
        }
    }
    
	StringBase(ConstCharPointer pStr)
		: m_pStringData(nullptr)
		, m_StringLength(0)
        , m_Capacity(0)
	{
		m_StringLength = CharLength(pStr);
        if(m_StringLength)
        {
            m_Capacity = (uint64)(1.5f * m_StringLength + 0.5f);
            m_pStringData = Allocate(m_Capacity);
            memcpy(m_pStringData, pStr, m_StringLength * sizeof(BaseChar));
            m_pStringData[m_StringLength] = 0;
        }
	}

	StringBase(const ThisString & rhs)
		: m_pStringData(nullptr)
		, m_StringLength(0)
        , m_Capacity(rhs.m_Capacity)
        , m_StringAllocator(rhs.m_StringAllocator)
	{
		Assign(rhs);
	}
	
	StringBase(ThisString && rhs)
		: m_pStringData(nullptr)
		, m_StringLength(0)
	{
		MoveAssign(Move(rhs));
	}

	virtual ~StringBase()
	{
		if (m_pStringData)
		{
			Deallocate();
			m_pStringData = nullptr;
			m_StringLength = 0;
		}
	}

	uint64				Length() const { return m_StringLength; }
	ConstCharPointer	Data() const { return m_pStringData; }
	ConstCharPointer	CStr() const { return m_pStringData; }

	ThisString&			operator=(const ThisString& rhs) { Assign(rhs); return *this; }
    ThisString&			operator=(ThisString&& rhs) { MoveAssign(Move(rhs)); return *this; }
    ThisString&         operator+=(const ThisString& rhs);
    ThisString&         AppendSprintf(const BaseChar* fmt, ...);
	void				Swap(ThisString& rhs);

    //CharPosition		FindFirstOf(BaseChar _char);

    template <typename T, typename A>
    friend Archive&     operator<<(Archive & ar, StringBase<T,A> const& str);
    
    template <typename T, typename A>
    friend Archive&     operator>>(Archive & ar, StringBase<T,A> & str);
    
protected:
	CharPointer			Allocate(uint64 stringLength);
	void				Deallocate();

	void				MoveAssign(ThisString && rhs);
	void				Assign(ThisString const& rhs);

private:
	CharPointer			m_pStringData;
	uint64				m_StringLength;
    uint64              m_Capacity;
	Allocator			m_StringAllocator;
};

template <typename BaseChar, typename Allocator>
KFORCE_INLINE BaseChar* StringBase<BaseChar, Allocator>::Allocate(uint64 length)
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
			Deallocate();
		}
		m_Capacity = rhs.m_StringLength + 1;
		m_pStringData = Allocate(m_Capacity);
		m_StringLength = rhs.m_StringLength;
		memcpy(m_pStringData, rhs.m_pStringData, rhs.m_StringLength * sizeof(BaseChar));
		m_pStringData[m_StringLength] = 0;
	}
}
    
template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>&
StringBase<BaseChar, Allocator>::AppendSprintf(const BaseChar *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int length = Vsnprintf(m_pStringData + m_StringLength, m_Capacity-m_StringLength, fmt, va);
    va_end(va);
    
    auto newLen = length + m_StringLength;
    if(newLen >= m_Capacity)
    {
        m_Capacity = (uint64)( 1.33f * newLen + 1.0f );
        BaseChar* newString = Allocate(m_Capacity);
        memcpy(newString, m_pStringData, m_StringLength*sizeof(BaseChar));
        
        va_list va;
        va_start(va, fmt);
        Vsnprintf(newString + m_StringLength, m_Capacity-m_StringLength, fmt, va);
        va_end(va);
        
        Deallocate();
        m_pStringData = newString;
    }
    
    m_StringLength = newLen;
    
    return *this;
}
    
template <typename BaseChar, typename Allocator>
KFORCE_INLINE void StringBase<BaseChar, Allocator>::Swap(StringBase<BaseChar, Allocator> & rhs)
{
	BaseChar* p = rhs.m_pStringData;
	rhs.m_pStringData = m_pStringData;
	m_pStringData = p;

	uint64 l = rhs.m_StringLength;
	rhs.m_StringLength = m_StringLength;
	m_StringLength = l;
}
    
template <typename BaseChar, typename Allocator>
KFORCE_INLINE StringBase<BaseChar, Allocator>&
StringBase<BaseChar, Allocator>::operator+=(StringBase<BaseChar, Allocator> const& rhs)
{
    auto rLen = rhs.m_StringLength;
    auto newLen = m_StringLength + rLen;
    if(newLen>=m_Capacity)
    {
        m_Capacity = uint64(1.5*newLen + 1);
        auto pNewData = Allocate(m_Capacity);
        if(m_pStringData)
        {
            memcpy(pNewData, m_pStringData, m_StringLength*sizeof(BaseChar));
            Deallocate();
        }
        m_pStringData = pNewData;
    }
    memcpy(m_pStringData + m_StringLength, rhs.m_pStringData, rLen*sizeof(BaseChar));
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
KFORCE_INLINE bool operator==(StringBase<BaseChar, Allocator> const& lhs, StringBase<BaseChar, Allocator> const& rhs)
{
    return ((lhs.Length() == rhs.Length()) && (memcmp(lhs.Data(), rhs.Data(), lhs.Length() * sizeof(BaseChar)) == 0));
}
    
template <typename BaseChar, typename Allocator>
Archive& operator<<(Archive & ar, StringBase<BaseChar, Allocator> const& str)
{
    ar << str.m_Capacity << str.m_StringLength;
    ar.ArrayIn(str.CStr(), str.Length());
    return ar;
}

template <typename BaseChar, typename Allocator>
Archive& operator>>(Archive & ar, StringBase<BaseChar, Allocator> & str)
{
    ar >> str.m_Capacity >> str.m_StringLength;
    str.m_pStringData = str.Allocate(str.m_Capacity);
    ar.ArrayOut(str.m_pStringData, str.Length());
    str.m_pStringData[str.m_StringLength] = 0;
    return ar;
}
    
typedef StringBase<char, kAllocator> String;
    
}
