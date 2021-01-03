/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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
		__forceinline T& operator*() const { { return *ptr_; } }
		__forceinline T* operator->() const { { return ptr_; } }
		explicit operator bool() const
		{
			return ptr_ != nullptr;
		}
		void swap(Ptr& Other)
		{
			T * const pValue = Other.ptr_;
			Other.ptr_ = ptr_;
			ptr_ = pValue;
		}
		Ptr& operator=(const Ptr& Other) {
			typedef Ptr<T> ThisType;
			if (&Other != this) {
				ThisType(Other).swap(*this);
			}
			return *this;
		}
		T* get() const { return ptr_; }
		T** getAddressOf() { return &ptr_;}
	private:
		T* ptr_;
	};
    template <class T, typename TResult>
    class ResultPtr : public Ptr<T>
    {
    public:
        ResultPtr(TResult in_result = TResult::Ok)
            : Ptr<T>()
            , result(in_result)
        {}

        ResultPtr(T* obj, TResult in_result = TResult::Ok)
            : Ptr<T>(obj)
            , result(in_result)
        {}

        TResult result;
    };

    class RenderHandle
    {
    public:
        // should be less than 15
        enum Type {
            Buffer,
            Texture,
            Sampler,
            RenderPass,
            Pipeline,
            BindGroup,
            BufferView,
            TextureView,
            Fence,
            // ..
            RTAccelerationStructure = 14,
            TypeMax = 15,
        };

        RenderHandle() : handle_(0) {}

        static RenderHandle allocate(Type type);
        static void         free(RenderHandle const& handle);

        inline Type         type() const { return static_cast<Type>(type_); }
        inline uint64_t     id() const { return id_; }
        inline uint64_t     handle() const { return handle_; }

        inline bool         operator==(RenderHandle const& other) const { return handle() == other.handle(); }

    private:
        union {
            struct {
                uint64_t type_ : 4;
                uint64_t id_ : 60;
            };
            uint64_t handle_;
        };
    };
}
#endif
