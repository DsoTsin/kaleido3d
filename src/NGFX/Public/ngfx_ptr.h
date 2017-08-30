#pragma once
#ifndef __ngfx_ptr_h__
#define __ngfx_ptr_h__
#include "ngfx_atomic.h"
namespace ngfx
{
	struct Rc 
	{
		Rc() {}
		virtual ~Rc() {}

		int32_t retain_internal()
		{
			return atomic_increment(&ref_int_);
		}
		int32_t release_internal()
		{
			auto c = atomic_decrement(&ref_int_);
			if (ref_int_ == 0)
			{
				delete this;
			}
			return c;
		}
		int32_t release()
		{
			auto c = atomic_decrement(&ref_ext_);
			if (ref_ext_ == 0)
			{
				release_internal();
			}
			return c;
		}
		int32_t retain()
		{
			return atomic_increment(&ref_ext_);
		}
	private:
		int32_t ref_int_ = 1;
		int32_t ref_ext_ = 1;
	};


	template <class T>
	class Ptr
	{
	public:
		explicit Ptr(T * pObj) : ptr_(pObj) {}
		Ptr(Ptr<T> const& Other)
			: ptr_(Other.ptr_) {
			if (ptr_)
				ptr_->retain();
		}
		Ptr() : ptr_(nullptr) {}
		~Ptr()
		{
			if (ptr_)
			{
				ptr_->release();
				ptr_ = nullptr;
			}
		}
		T& operator*() const { { return *ptr_; } }
		T* operator->() const { { return ptr_; } }
		explicit operator bool() const
		{
			return ptr_ != nullptr;
		}
		void Swap(Ptr& Other)
		{
			T * const pValue = Other.ptr_;
			Other.ptr_ = ptr_;
			ptr_ = pValue;
		}
		Ptr& operator=(const Ptr& Other) {
			typedef Ptr<T> ThisType;
			if (&Other != this) {
				ThisType(Other).Swap(*this);
			}
			return *this;
		}
		T* Get() const { return ptr_; }
		T** GetAddressOf() { return &ptr_;}
	private:
		T* ptr_;
	};
}
#endif
