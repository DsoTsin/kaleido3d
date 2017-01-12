#pragma once

#include <Config/Config.h>
#include <Config/OSHeaders.h>
#include "TypeTrait.hpp"

extern K3D_API void* __k3d_malloc__(size_t sizeOfObj);
extern K3D_API void __k3d_free__(void *p, size_t sizeOfObj);

extern K3D_API void* operator new[](size_t size, const char* pName);

K3D_COMMON_NS
{
	template<typename U>
	struct DefaultDeletor
	{
		DefaultDeletor() = default;
		void operator()(U * pObj)
		{
			delete pObj;
		}
	};

	template<typename U>
	struct DefaultDeletor<U[]>
	{
		DefaultDeletor() = default;
		void operator()(U * pObj)
		{
			delete[] pObj;
		}
	};

	class kAllocator
	{
	public:
		kAllocator(const char* = nullptr) {}
		kAllocator(const kAllocator&) {}
		kAllocator(const kAllocator&, const char*) {}
		kAllocator& operator=(const kAllocator&) { return *this; }
		bool operator==(const kAllocator&) { return true; }
		bool operator!=(const kAllocator&) { return false; }
		void* allocate(size_t n, int /*flags = 0*/) { return __k3d_malloc__(n); }
		void* allocate(size_t n, size_t /*alignment*/, size_t /*alignmentOffset*/, int /*flags = 0*/)
		{
			return __k3d_malloc__(n);
		}
		void deallocate(void* p, size_t n) { __k3d_free__(p, n); }
		const char* get_name() const { return "kAllocator"; }
		void set_name(const char*) {}
	};
}
