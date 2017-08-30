#pragma once

#include "RefCount.h"

#if K3DPLATFORM_OS_WIN && ENABLE_SHAREDPTR_TRACKER
#include <Windows.h>
#include "String.hpp"
#endif

namespace k3d
{
	template <typename T> class WeakPtr;
	template <typename T> class SharedPtr;
	template <typename T> class EnableSharedFromThis;

	template <typename T, bool threadSafe>
	struct TSharedPtrBase
	{

	};

	template <typename T>
	class SharedPtr_MT : TSharedPtrBase<T, true>
	{

	};

	template <typename T, typename U>
	void __EnableSharedFromThis(const RefCountBase* pRefCount, const EnableSharedFromThis<T>* pEnableSharedFromThis, const U* pValue)
	{
		if (pEnableSharedFromThis)
			pEnableSharedFromThis->m_WeakPtr.Assign(const_cast<U*>(pValue), const_cast<RefCountBase*>(pRefCount));
	}

	inline void __EnableSharedFromThis(const RefCountBase*, ...)
	{
	}

	template <typename T>
	class SharedPtr : TSharedPtrBase<T, false>
	{
	public:
		typedef SharedPtr<T> ThisType;

		SharedPtr() : m_pValue(nullptr), m_pRefCount(nullptr) {}

		SharedPtr(const SharedPtr& sharedPtr)
			: m_pValue(sharedPtr.m_pValue)
			, m_pRefCount(sharedPtr.m_pRefCount)
		{
			if (m_pRefCount)
			{
				m_pRefCount->AddRef();

#if K3DPLATFORM_OS_WIN && ENABLE_SHAREDPTR_TRACKER
				String debugStr;
				debugStr.AppendSprintf("SharedPtr Track (Assign Construct) [%s] --- Strong=%d Weak=%d .\n",
					typeid(T).name(), m_pRefCount->m_RefCount, m_pRefCount->m_WeakRefCount);
				OutputDebugStringA(debugStr.CStr());
#endif
			}
		}

		SharedPtr(decltype(nullptr))
			: m_pValue(nullptr)
			, m_pRefCount(nullptr)
		{
		}

		template <typename U>
		SharedPtr(const SharedPtr<U>& sharedPtr)
			: m_pValue(sharedPtr.m_pValue)
			, m_pRefCount(sharedPtr.m_pRefCount)
		{
			if (m_pRefCount)
			{
				m_pRefCount->AddRef();

#if K3DPLATFORM_OS_WIN && ENABLE_SHAREDPTR_TRACKER
				String debugStr;
				debugStr.AppendSprintf("SharedPtr Track (Assign Construct) [%s] --- Strong=%d Weak=%d .\n",
					typeid(U).name(), m_pRefCount->m_RefCount, m_pRefCount->m_WeakRefCount);
				OutputDebugStringA(debugStr.CStr());
#endif
			}
		}

		template <typename U>
		SharedPtr(const SharedPtr<U>& sharedPtr, T* pValue)
			: m_pValue(pValue)
			, m_pRefCount(sharedPtr.m_pRefCount)
		{
			if (m_pRefCount)
				m_pRefCount->AddRef();
		}

		template <typename U>
		explicit SharedPtr(U* pValue)
			: m_pValue(nullptr)
			, m_pRefCount(nullptr)
		{
			AllocInternal(pValue, DefaultDeletor<U>());
		}

		template <typename U>
		explicit SharedPtr(const WeakPtr<U>& weakPtr)
			: m_pValue(weakPtr.m_pValue),
			  m_pRefCount(weakPtr.m_pRefCount ? weakPtr.m_pRefCount->Lock() : weakPtr.m_pRefCount)
		{
			if(!m_pRefCount)
			{
				m_pValue = nullptr;
			}
		}

		~SharedPtr()
		{
			if (m_pRefCount) 
			{
				I32 RefCount = m_pRefCount->Release();
#if K3DPLATFORM_OS_WIN && ENABLE_SHAREDPTR_TRACKER
				String debugStr;
				debugStr.AppendSprintf("SharedPtr Track (Release) [%s] --- Strong=%d Weak=%d .\n", 
          typeid(T).name(), m_pRefCount->m_RefCount, RefCount);
				OutputDebugStringA(debugStr.CStr());
#else
        (void)RefCount;
#endif
			}
			m_pValue = nullptr;
			m_pRefCount = nullptr;
		}

		int UseCount() const
		{
			return m_pRefCount ? m_pRefCount->m_RefCount : 0;
		}

		void Swap(SharedPtr& sharedPtr)
		{
			T * const pValue = sharedPtr.m_pValue;
			sharedPtr.m_pValue = m_pValue;
			m_pValue = pValue;
			RefCountBase* const pRefCount = sharedPtr.m_pRefCount;
			sharedPtr.m_pRefCount = m_pRefCount;
			m_pRefCount = pRefCount;
		}

        void ForceAddRef()
        {
            m_pRefCount->AddRef();
        }

        void ForceRelease()
        {
            m_pRefCount->Release();
        }

		void Reset()
		{
			ThisType().Swap(*this);
		}

		KFORCE_INLINE T& operator*() const { return *m_pValue; }
        KFORCE_INLINE T* operator->() const {	return m_pValue; }

		SharedPtr& operator=(const SharedPtr& sharedPtr)
		{
			if(&sharedPtr != this)
			{
				ThisType(sharedPtr).Swap(*this);
			}

#if K3DPLATFORM_OS_WIN && ENABLE_SHAREDPTR_TRACKER
			String debugStr;
			debugStr.AppendSprintf("SharedPtr Track (Assign) [%s] --- Strong=%d Weak=%d .\n",
				typeid(T).name(), m_pRefCount->m_RefCount, m_pRefCount->m_WeakRefCount);
			OutputDebugStringA(debugStr.CStr());
#endif
			return *this;
		}

		explicit operator bool() const
		{
			return m_pValue != nullptr;
		}

        KFORCE_INLINE T* Get() const { return m_pValue; }

	protected:
		T*				m_pValue;
		RefCountBase*	m_pRefCount;

		template <typename U> friend class SharedPtr;
		template <typename U> friend class WeakPtr;
		template <typename U, typename... Args> friend SharedPtr<U> MakeShared(Args&&... args);
    template <typename U, typename... Args>
    friend SharedPtr<U> MakeSharedWithInfo(const char* _ClassName, const char* _Source, int _SourceLine, Args&&... args);
	private:
		template <typename U, typename Deleter>
		void AllocInternal(U* pValue, Deleter deleter)
		{
			typedef TRefCount<U*, Deleter> RefCountT;
			void* const pMemory = k3d_malloc(sizeof(RefCountT));
			if(pMemory)
			{
				m_pRefCount = ::new(pMemory) RefCountT(pValue, Move(deleter));
				m_pValue = pValue;
				__EnableSharedFromThis(m_pRefCount, pValue, pValue);
			}
			else
			{
				deleter(pValue);
			}
		}
	};

#define MakeSharedMacro(_ClassName, ...) MakeSharedWithInfo<_ClassName>(#_ClassName, __FILE__, __LINE__, __VA_ARGS__)

  template <typename T, typename... Args>
  SharedPtr<T> MakeSharedWithInfo(const char* _ClassName, const char* _Source, int _SourceLine, Args&&... args)
  {
    typedef TRefCountInstance<T> RCT;
    SharedPtr<T> sharedPtr;
    RCT* pRefCount = ::new(_ClassName, _Source, _SourceLine) RCT(Forward<Args>(args)...);
    sharedPtr.m_pRefCount = (RefCountBase*)pRefCount;
    sharedPtr.m_pValue = pRefCount->GetValue();
    __EnableSharedFromThis(pRefCount, pRefCount->GetValue(), pRefCount->GetValue());
    return sharedPtr;
  }

	template <typename T, typename... Args>
	SharedPtr<T> MakeShared(Args&&... args)
	{
		typedef TRefCountInstance<T> RCT;
		SharedPtr<T> sharedPtr;
		void* const pMemory = k3d_malloc(sizeof(RCT));
		if(pMemory)
		{
			RCT* pRefCount = ::new(pMemory) RCT(Forward<Args>(args)...);
			sharedPtr.m_pRefCount = (RefCountBase*)pRefCount;
			sharedPtr.m_pValue = pRefCount->GetValue();
			__EnableSharedFromThis(pRefCount, pRefCount->GetValue(), pRefCount->GetValue());
		}
		return sharedPtr;
	}

	template <typename T, typename U>
	inline SharedPtr<T> StaticPointerCast(const SharedPtr<U> &sharedPtr)
	{
		return SharedPtr<T>(sharedPtr, static_cast<T*>(sharedPtr.Get()));
	}

	template <typename T, typename U>
	inline SharedPtr<T> DynamicPointerCast(const SharedPtr<U>& sharedPtr)
	{
		return SharedPtr<T>(sharedPtr, dynamic_cast<T*>(sharedPtr.Get()));
	}

	template <typename T>
	class WeakPtr
	{
	public:
		typedef WeakPtr<T> ThisType;

		WeakPtr(decltype(nullptr))
			: m_pValue(nullptr)
			, m_pRefCount(nullptr)
		{
		}

		WeakPtr() : m_pValue(nullptr), m_pRefCount(nullptr) {}

		template <typename U>
		WeakPtr(const SharedPtr<U>& sharedPtr)
			: m_pValue(sharedPtr.m_pValue)
			, m_pRefCount(sharedPtr.m_pRefCount)
		{
			if(m_pRefCount)
				m_pRefCount->AddWeakRef();
		}

		~WeakPtr() 
		{
			if(m_pRefCount)
				m_pRefCount->ReleaseWeakRef();
		}

		T& operator*() const { return *m_pValue; }
		
		T* operator->() const { return m_pValue; }

		void Swap(WeakPtr& weakPtr)
		{
			T * const pValue = weakPtr.m_pValue;
			weakPtr.m_pValue = m_pValue;
			m_pValue = pValue;
			RefCountBase* const pRefCount = weakPtr.m_pRefCount;
			weakPtr.m_pRefCount = m_pRefCount;
			m_pRefCount = pRefCount;
		}

		WeakPtr& operator=(const WeakPtr& weakPtr)
		{
			if (&weakPtr != this)
			{
				WeakPtr<T>(weakPtr).Swap(*this);
			}
			return *this;
		}

		explicit operator bool() const
		{
			return m_pValue != nullptr;
		}

		T* Get() const { return m_pValue; }

		void Assign(T* pValue, RefCountBase* pRefCount)
		{
			m_pValue = pValue;

			if (pRefCount != m_pRefCount)
			{
				if (m_pRefCount)
					m_pRefCount->ReleaseWeakRef();

				m_pRefCount = pRefCount;

				if (m_pRefCount)
					m_pRefCount->AddWeakRef();
			}
		}

	protected:
		T*				m_pValue;
		RefCountBase* 	m_pRefCount;

		template <typename U> friend class SharedPtr;
		template <typename U> friend class WeakPtr;
		template <typename U> friend class EnableSharedFromThis;
	private:
	};

	template<typename T> class EnableSharedFromThis
	{
	protected:
		template <typename U> friend class SharedPtr;
		EnableSharedFromThis() {}
		EnableSharedFromThis(EnableSharedFromThis const &) {}
		EnableSharedFromThis & operator=(EnableSharedFromThis const &)
		{
			return *this;
		}
		~EnableSharedFromThis()
		{
		}
	public:
		SharedPtr<T> SharedFromThis()
		{
			return SharedPtr<T>(m_WeakPtr);
		}
		SharedPtr<T const> SharedFromThis() const
		{
			return SharedPtr<T const>(m_WeakPtr);
		}
	public:
		mutable WeakPtr<T> m_WeakPtr;
	};
}