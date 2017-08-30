#ifndef __DynArray_hpp__
#define __DynArray_hpp__
#pragma once

#include "Archive.h"

#if (K3DPLATFORM_OS_WIN) || (K3DPLATFORM_OS_ANDROID)
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include <string.h>

namespace k3d
{
  template<class T,
           bool hasConstructor =
             (__has_trivial_constructor(T) || __has_nothrow_constructor(T)) &&
             !__is_pod(T)>
  struct __Initializer
  {
    static void DoInit(T* begin, T* end)
    {
      for (T* iter = begin; iter != end; iter++) {
        new (iter) T();
      }
    }

    static void DoInitWithValue(T* begin, T* end, T const& value)
    {
      for (T* iter = begin; iter != end; iter++) {
        new (iter) T(value);
      }
    }
  };

  template<class T>
  struct __Initializer<T, false>
  {
    static void DoInit(T* begin, T* end)
    {
      memset(begin, 0, (end - begin) * sizeof(T));
    }

    static void DoInitWithValue(T* begin, T* end, T const& value)
    {
      for (T* iter = begin; iter != end; iter++) {
        new (iter) T(value);
      }
    }
  };

  template<class T, bool isClass = __is_class(T)>
  struct __Copier
  {
    static void DoCopy(T* dest, T* src, size_t n);
  };

  template<class T>
  struct __Copier<T, true>
  {
    static void DoCopy(T* dest, T* src, size_t n)
    {
      for (T* iter = dest; size_t(iter - dest) != n; iter++) {
        *iter = *src++;
      }
    }
  };

  template<class T>
  struct __Copier<T, false>
  {
    static void DoCopy(T* dest, T* src, size_t n)
    {
      ::memcpy(dest, src, sizeof(T) * n);
    }
  };

  template<typename ElementType, typename TAllocator = kAllocator, int PreAlloc = 4>
  class DynArray
  {
  public:
    DynArray() K3D_NOEXCEPT
      : m_ElementIndex(0)
      , m_ElementCount(0)
      , m_Capacity(PreAlloc)
      , m_pElement(nullptr)
    {
      m_pElement =
        (ElementType*)m_Allocator.allocate(m_Capacity * sizeof(ElementType), 0);
      __Initializer<ElementType>::DoInit(m_pElement, m_pElement + m_Capacity);
    }

    DynArray(U64 capacity) K3D_NOEXCEPT
      : m_ElementIndex(0)
      , m_ElementCount(0)
      , m_Capacity(capacity)
      , m_pElement(nullptr)
    {
      m_pElement =
        (ElementType*)m_Allocator.allocate(m_Capacity * sizeof(ElementType), 0);
      __Initializer<ElementType>::DoInit(m_pElement, m_pElement + m_Capacity);
    }

    DynArray(U64 size, ElementType const& value) K3D_NOEXCEPT
      : m_ElementIndex(size-1)
      , m_ElementCount(size)
      , m_Capacity(size)
      , m_pElement(nullptr)
    {
      m_pElement =
        (ElementType*)m_Allocator.allocate(m_Capacity * sizeof(ElementType), 0);
      __Initializer<ElementType>::DoInitWithValue(m_pElement, m_pElement + m_Capacity, value);
    }

    DynArray(DynArray&& rhs)
      : m_ElementCount(0)
      , m_ElementIndex(0)
      , m_pElement(nullptr)
    {
      m_ElementCount = rhs.m_ElementCount;
      m_Capacity = rhs.m_Capacity;
      m_pElement = rhs.m_pElement;
      rhs.m_pElement = nullptr;
      rhs.m_Capacity = 0;
      rhs.m_ElementCount = 0;
    }

    DynArray(DynArray&& rhs, TAllocator& alloc)
      : m_ElementCount(0)
      , m_ElementIndex(0)
      , m_pElement(nullptr)
      , m_Allocator(alloc)
    {
      m_ElementCount = rhs.m_ElementCount;
      m_Capacity = rhs.m_Capacity;
      m_pElement = rhs.m_pElement;
    }

    DynArray(DynArray const& rhs)
      : m_ElementIndex(0)
      , m_ElementCount(0)
      , m_pElement(nullptr)
    {
      m_ElementCount = rhs.m_ElementCount;
      m_Capacity = rhs.m_Capacity;
      m_pElement =
        (ElementType*)m_Allocator.allocate(m_Capacity * sizeof(ElementType), 0);
      __Initializer<ElementType>::DoInit(m_pElement, m_pElement + m_Capacity);
      __Copier<ElementType>::DoCopy(
        m_pElement, rhs.m_pElement, rhs.m_ElementCount);
    }

    template<typename OtherElementType>
    DynArray(OtherElementType* data, U32 count)
      : m_ElementIndex(0)
      , m_pElement(nullptr)
    {
      m_Capacity =
        (U64)(count * sizeof(OtherElementType) / sizeof(ElementType) * 1.5f);
      m_ElementCount = count * sizeof(OtherElementType) / sizeof(ElementType);
      m_pElement =
        (ElementType*)m_Allocator.allocate(m_Capacity * sizeof(ElementType), 0);
      memcpy(m_pElement, data, count * sizeof(OtherElementType));
    }

    ~DynArray()
    {
      if (m_pElement) {
        Deconstruct();
        m_Allocator.deallocate(m_pElement, 0);
        m_pElement = nullptr;
      }
    }

    void Deconstruct()
    {
      for (U64 i = 0; i < m_ElementCount; i++) {
        typedef ElementType ElementTypeType;
        m_pElement[i].ElementTypeType::~ElementTypeType();
      }
    }

    DynArray& Append(ElementType const& element)
    {
      m_ElementCount++;
      if (m_ElementCount == m_Capacity) {
        ReAdjust(m_Capacity * 2);
      }
      m_pElement[m_ElementCount - 1] = element;
      return *this;
    }

    DynArray& AddAll(DynArray<ElementType> const& rhs)
    {
      auto merged = rhs.Count() + m_ElementCount;
      if (merged >= m_Capacity) {
        ReAdjust(merged + 1);
      }
      __Copier<ElementType>::DoCopy(
        m_pElement + m_ElementCount, rhs.m_pElement, rhs.m_ElementCount);
      m_ElementCount = merged;
      return *this;
    }

    DynArray& Append(ElementType&& element)
    {
      if (m_ElementCount == m_Capacity) {
        ReAdjust(m_Capacity * 2);
      }
      m_ElementCount++;
      m_pElement[m_ElementCount - 1] = element;
      return *this;
    }

    DynArray& operator=(DynArray const& rhs)
    {
      m_ElementCount = rhs.m_ElementCount;
      m_Capacity = rhs.m_Capacity;
      if (m_pElement) {
        m_Allocator.deallocate(m_pElement, 0);
      }
      m_pElement =
        (ElementType*)m_Allocator.allocate(m_Capacity * sizeof(ElementType), 0);
      __Initializer<ElementType>::DoInit(m_pElement, m_pElement + m_Capacity);
      __Copier<ElementType>::DoCopy(
        m_pElement, rhs.m_pElement, rhs.m_ElementCount);
      return *this;
    }

    void Swap(DynArray& rhs)
    {
      {
        ElementType* tmp = rhs.m_pElement;
        rhs.m_pElement = m_pElement;
        m_pElement = tmp;
      }
      {
        U32 tmpCount = rhs.m_ElementCount;
        rhs.m_ElementCount = m_ElementCount;
        m_ElementCount = tmpCount;
      }
      {
        U32 tmp = rhs.m_Capacity;
        rhs.m_Capacity = m_Capacity;
        m_Capacity = tmp;
      }
      {
        TAllocator tmp = rhs.m_Allocator;
        rhs.m_Allocator = m_Allocator;
        m_Allocator = tmp;
      }
    }

    void Clear()
    {
      Deconstruct();
      m_ElementCount = 0;
      m_ElementIndex = 0;
    }

    void Resize(int NewElementCount)
    {
      if (NewElementCount > m_Capacity) {
        ElementType* pElement = (ElementType*)m_Allocator.allocate(
          NewElementCount * sizeof(ElementType), 0);
        __Initializer<ElementType>::DoInit(pElement,
                                           pElement + NewElementCount);
        if (m_ElementCount > 0) {
          __Copier<ElementType>::DoCopy(pElement, m_pElement, m_ElementCount);
          m_Allocator.deallocate(m_pElement, 0);
        }
        m_Capacity = NewElementCount;
        m_pElement = pElement;
      }
      m_ElementCount = NewElementCount;
    }

    ElementType const& operator[](U64 index) const
    {
      return m_pElement[index];
    }

    ElementType& operator[](U64 index) { return m_pElement[index]; }

    ElementType* Data() { return m_pElement; }

    ElementType const* Data() const { return m_pElement; }

    U64 Count() const { return m_ElementCount; }
    U64 Capacity() const { return m_Capacity; }

    bool Contains(ElementType const& item) const
    {
      for (auto iter = begin(); iter != end(); ++iter) {
        if (*iter == item) {
          return true;
        }
      }
      return false;
    }

    typedef ElementType ValueType;
    typedef ValueType*  Iterator;

#ifndef DISABLE_STD_INTERFACE

    typedef ElementType value_type;
    typedef value_type* iterator;
    typedef value_type const* const_iterator;

    const_iterator begin() const { return m_pElement; }

    const_iterator end() const { return m_pElement + m_ElementCount; }

    iterator begin() { return m_pElement; }

    iterator end() { return m_pElement + m_ElementCount; }

    bool empty() const { return m_ElementCount == 0; }
#endif

  private:
    template<typename T>
    friend Archive& operator<<(Archive& ar, DynArray<T> const& rhs);
    template<typename T>
    friend Archive& operator>>(Archive& ar, DynArray<T>& rhs);

    void ReAdjust(U64 NewElementCount)
    {
      ElementType* pElement = (ElementType*)m_Allocator.allocate(
        NewElementCount * sizeof(ElementType), 0);
      __Initializer<ElementType>::DoInit(pElement, pElement + NewElementCount);
      __Copier<ElementType>::DoCopy(pElement, m_pElement, m_ElementCount);
      m_Capacity = NewElementCount;
      m_Allocator.deallocate(m_pElement, 0);
      m_pElement = pElement;
    }

    U64 m_ElementIndex;
    U64 m_ElementCount;
    U64 m_Capacity;
    ElementType* m_pElement;
    TAllocator m_Allocator;
  };

  template<typename T>
  inline Archive& operator<<(Archive& ar, DynArray<T> const& rhs)
  {
    ar << rhs.m_ElementCount << rhs.m_Capacity;
    for (auto ele : rhs) {
      ar << ele;
    }
    return ar;
  }

  template<typename T>
  inline Archive& operator>>(Archive& ar, DynArray<T>& rhs)
  {
    ar >> rhs.m_ElementCount >> rhs.m_Capacity;
    if (rhs.m_pElement) {
      rhs.m_Allocator.deallocate(rhs.m_pElement, 0);
    }
    rhs.m_pElement =
      (T*)rhs.m_Allocator.allocate(rhs.m_Capacity * sizeof(T), 0);
    for (U32 i = 0; i < rhs.m_ElementCount; i++) {
      ar >> rhs.m_pElement[i];
    }
    return ar;
  }
}

#endif
