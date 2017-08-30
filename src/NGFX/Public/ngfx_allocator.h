#pragma once
#ifndef __ngfx_allocator_h__
#define __ngfx_allocator_h__

extern "C" void k3d_free(void * ptr, size_t size);

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
