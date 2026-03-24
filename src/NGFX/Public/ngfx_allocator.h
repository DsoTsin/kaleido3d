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
#ifndef __ngfx_allocator_h__
#define __ngfx_allocator_h__

#include <stddef.h>

#ifndef BUILD_SHARED_LIB
#define BUILD_SHARED_LIB 0
#endif

#if BUILD_SHARED_LIB
#if _MSC_VER
#define NGFX_API __declspec(dllexport)
#else
#define NGFX_API __attribute__((visibility("default"))) 
#endif
#else
#define NGFX_API __declspec(dllimport)
#endif

extern "C" NGFX_API void k3d_free(void * ptr, size_t size);

namespace ngfx
{
    template <typename T>
    struct DefaultDeleter
    {
        constexpr DefaultDeleter() = default;
        void operator() (T* InPtr) const 
        {
            delete InPtr;
        }
    };

    template <typename T, class TDeleter = DefaultDeleter<T> >
    class UniqPtr
    {
    public:
        UniqPtr(T* InPtr = nullptr) : m_Ptr(InPtr)
        {}

        UniqPtr(UniqPtr&& Other) : m_Ptr(nullptr)
        {
            T* OtherPtr = Other.m_Ptr;
            auto Deleter = Other.m_Deleter;
            m_Ptr = OtherPtr;
            m_Deleter = Deleter;
            Other.m_Ptr = nullptr;
        }
        ~UniqPtr()
        {
            if (m_Ptr) 
            {
                m_Deleter(m_Ptr);
                m_Ptr = nullptr;
            }
        }

        T* operator->() const { return m_Ptr; }

        void reset(T* InPtr = nullptr)
        {
            if (m_Ptr != InPtr) 
            {
                if (m_Ptr) 
                {
                    m_Deleter(m_Ptr);
                }
                m_Ptr = InPtr;
            }
        }
        
        explicit operator bool() const
        {
            return m_Ptr != nullptr;
        }

        UniqPtr(const UniqPtr&) = delete;
        UniqPtr& operator=(const UniqPtr&) = delete;
    private:
        T* m_Ptr;
        TDeleter m_Deleter;
    };
}

#endif
