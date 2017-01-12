#pragma once

#include "RefCount.hpp"

K3D_COMMON_NS
{
	template <typename T, bool threadSafe>
	struct TSharedPtrBase
	{

	};

	template <typename T>
	class SharedPtr_MT : TSharedPtrBase<T, true>
	{

	};

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
				m_pRefCount->AddRef();
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
				m_pRefCount->AddRef();
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

		~SharedPtr()
		{
			if (m_pRefCount)
				m_pRefCount->Release();
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

		void Reset()
		{
			ThisType().Swap(*this);
		}

		T& operator*() const { return *m_pValue; }
		T* operator->() const {	return m_pValue; }

		SharedPtr& operator=(const SharedPtr& sharedPtr)
		{
			if(&sharedPtr != this)
			{
				ThisType(sharedPtr).Swap(*this);
			}
			return *this;
		}

		explicit operator bool() const
		{
			return m_pValue != nullptr;
		}

		T* Get() const { return m_pValue; }

	protected:
		T*				m_pValue;
		RefCountBase*	m_pRefCount;

		template <typename U> friend class SharedPtr;
		template <typename U> friend class WeakPtr;
		template <typename U, typename... Args> friend SharedPtr<U> MakeShared(Args&&... args);

	private:
		template <typename U, typename Deleter>
		void AllocInternal(U* pValue, Deleter deleter)
		{
			typedef TRefCount<U*, Deleter> RefCountT;
			void* const pMemory = __k3d_malloc__(sizeof(RefCountT));
			if(pMemory)
			{
				m_pRefCount = ::new(pMemory) RefCountT(pValue, Move(deleter));
				m_pValue = pValue;
			}
			else
			{
				deleter(pValue);
			}
		}
	};

	template <typename T, typename... Args>
	SharedPtr<T> MakeShared(Args&&... args)
	{
		typedef TRefCountInstance<T> RCT;
		SharedPtr<T> sharedPtr;
		void* const pMemory = __k3d_malloc__(sizeof(RCT));
		if(pMemory)
		{
			RCT* pRefCount = ::new(pMemory) RCT(Forward<Args>(args)...);
			sharedPtr.m_pRefCount = (RefCountBase*)pRefCount;
			sharedPtr.m_pValue = pRefCount->GetValue();
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

	protected:
		T*				m_pValue;
		RefCountBase* 	m_pRefCount;

		template <typename U> friend class SharedPtr;
		template <typename U> friend class WeakPtr;
	};
}