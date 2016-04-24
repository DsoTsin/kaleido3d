#ifndef __DynArray_hpp__
#define __DynArray_hpp__
#pragma once

#include "Config/PlatformTypes.h"
#include <malloc.h>

#ifdef DYNARRAY_TEST_CASE
#include <iostream>
typedef unsigned int uint32;
typedef size_t uint64;
#endif

namespace k3d
{
	class DefaultAllocator
	{
	public:
		DefaultAllocator()
		{
		}

		~DefaultAllocator()
		{
		}

		void *Allocate(size_t szData)
		{
			return malloc(szData);
		}

		void DeAllocate(void* pData)
		{
			free(pData);
		}
	};

	template <typename ElementType, typename TAllocator = DefaultAllocator>
	class DynArray
	{
	public:
		DynArray() : m_ElementCount(0), m_ElementIndex(0), m_Capacity(4), m_pElement(nullptr)
		{
			m_pElement = (ElementType*)m_Allocator.Allocate(m_Capacity*sizeof(ElementType));
			//memset(m_pElement, 0, m_Capacity*sizeof(ElementType));
		}

		DynArray(DynArray && rhs) : m_ElementCount(0), m_ElementIndex(0), m_pElement(nullptr)
		{
			m_ElementCount = rhs.m_ElementCount;
			m_Capacity = rhs.m_Capacity;
			m_pElement = rhs.m_pElement;
		}

		DynArray(DynArray && rhs, TAllocator & alloc) : m_ElementCount(0), m_ElementIndex(0), m_pElement(nullptr), m_Allocator(alloc)
		{
			m_ElementCount = rhs.m_ElementCount;
			m_Capacity = rhs.m_Capacity;
			m_pElement = rhs.m_pElement;
		}

		DynArray(DynArray const& rhs) : m_ElementCount(0), m_ElementIndex(0), m_pElement(nullptr)
		{
			m_ElementCount = rhs.m_ElementCount;
			m_Capacity = rhs.m_Capacity;
			m_pElement = (ElementType*)m_Allocator.Allocate(m_Capacity*sizeof(ElementType));
			memcpy(m_pElement, rhs.m_pElement, rhs.m_ElementCount * sizeof(ElementType));
		}

		template <typename OtherElementType>
		DynArray(OtherElementType * data, uint32 count) : m_ElementIndex(0), m_pElement(nullptr)
		{
			m_Capacity = (uint32)(count * sizeof(OtherElementType) / sizeof(ElementType) * 1.5f);
			m_ElementCount = count * sizeof(OtherElementType) / sizeof(ElementType);
			m_pElement = (ElementType*)m_Allocator.Allocate(m_Capacity * sizeof(ElementType));
			memcpy(m_pElement, data, count * sizeof(OtherElementType));
		}

		~DynArray()
		{
			if (m_pElement)
			{
				Deconstruct();
				m_Allocator.DeAllocate(m_pElement);
				m_pElement = nullptr;
			}
		}

		/*void Construct()
		{
		for (uint32 i = 0; i < m_ElementCount; i++)
		{
		typedef ElementType ElementTypeType;
		m_pElement[i].ElementTypeType::ElementTypeType();
		}
		}*/

		void Deconstruct()
		{
			for (uint32 i = 0; i < m_ElementCount; i++)
			{
				typedef ElementType ElementTypeType;
				m_pElement[i].ElementTypeType::~ElementTypeType();
			}
		}

		DynArray& Append(ElementType const & element)
		{
			m_ElementCount++;
			if (m_ElementCount == m_Capacity)
			{
				ReAdjust(m_Capacity * 2);
			}
			m_pElement[m_ElementCount - 1] = element;
			return *this;
		}

		DynArray& Append(ElementType && element)
		{
			m_ElementCount++;
			if (m_ElementCount == m_Capacity)
			{
				ReAdjust(m_Capacity * 2);
			}
			m_pElement[m_ElementCount - 1] = element;
			return *this;
		}

		DynArray& operator=(DynArray const& rhs)
		{
			m_ElementCount = rhs.m_ElementCount;
			m_Capacity = rhs.m_Capacity;
			if (m_pElement)
			{
				m_Allocator.DeAllocate(m_pElement);
			}
			m_pElement = (ElementType*)m_Allocator.Allocate(m_Capacity*sizeof(ElementType));
			memcpy(m_pElement, rhs.m_pElement, rhs.m_ElementCount * sizeof(ElementType));
			return *this;
		}

		void Swap(DynArray & rhs)
		{
			{
				ElementType *tmp = rhs.m_pElement;
				rhs.m_pElement = m_pElement;
				m_pElement = tmp;
			}
			{
				uint32 tmpCount = rhs.m_ElementCount;
				rhs.m_ElementCount = m_ElementCount;
				m_ElementCount = tmpCount;
			}
			{
				uint32 tmp = rhs.m_Capacity;
				rhs.m_Capacity = m_Capacity;
				m_Capacity = tmp;
			}
			{
				TAllocator tmp = rhs.m_Allocator;
				rhs.m_Allocator = m_Allocator;
				m_Allocator = tmp;
			}
		}

		ElementType const& operator[](uint32 index) const
		{
			return m_pElement[index];
		}


		ElementType& operator[](uint32 index)
		{
			return m_pElement[index];
		}

		ElementType* Data()
		{
			return m_pElement;
		}

		ElementType const* Data() const
		{
			return m_pElement;
		}

		uint32 Count() const
		{
			return m_ElementCount;
		}

	private:

		void ReAdjust(uint32 NewElementCount)
		{
			ElementType* pElement = (ElementType*)m_Allocator.Allocate(NewElementCount*sizeof(ElementType));
			memmove(pElement, m_pElement, m_ElementCount*sizeof(ElementType));
			m_Capacity = NewElementCount;
			m_Allocator.DeAllocate(m_pElement);
			m_pElement = pElement;

#ifdef DYNARRAY_TEST_CASE
			std::cerr << "Need Reallocate .. capacity=" << m_Capacity << std::endl;
#endif
		}

		uint32			m_ElementIndex;
		uint32			m_ElementCount;
		uint32			m_Capacity;
		ElementType *	m_pElement;
		TAllocator		m_Allocator;
	};

}

#ifdef DYNARRAY_TEST_CASE
using namespace k3d;

struct B
{
	B() : msg_(nullptr)
	{
		std::cout << "construct" << std::endl;
	}
	B(const char* msg) : msg_(msg)
	{
		std::cout << msg << std::endl;
	}

	B& operator=(const B& rhs)
	{
		std::cout << "rhs " << rhs.msg_
			<< " assigned to " << std::endl;
		this->msg_ = rhs.msg_;
		return *this;
	}

	~B()
	{
		std::cout << "deconstructB " << msg_ << std::endl;
	}
	const char *msg_;
		};

B b1{ "b1" }, b3{ "b3" };
DynArray<int> ints;
int main()
{
	ints.Append(5).Append(6).Append(7).Append(0).Append(8);
	std::cout << ints[3] << std::endl;
	ints[3] = 0;
	std::cout << ints[3] << std::endl;
	std::cout << ints.Count() << std::endl;
	{
		B b2{ "b2" };
		DynArray<B> bs;
		bs.Append(b1)
			.Append(std::move(b2))
			.Append(b3);
		bs[2] = B("B modified");
	}
	{
		DynArray<B> bs;
		bs.Append(B("A1")).Append(B("A2")).Append(B("A3")).Append(B("A4"));
	}
	system("pause");
	return 0;
}

#endif
#endif