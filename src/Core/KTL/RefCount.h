#pragma once

namespace k3d
{
	struct RefCountBase
	{
		I32 m_RefCount;
		I32 m_WeakRefCount;

	public:
		RefCountBase(I32 refCount = 1, I32 weakRefCount = 1) K3D_NOEXCEPT
			: m_RefCount(refCount), m_WeakRefCount(weakRefCount) {}

		virtual ~RefCountBase() K3D_NOEXCEPT {}

		I32 UseCount() const K3D_NOEXCEPT { return m_RefCount; }

		I32 AddRef() K3D_NOEXCEPT
		{
            __intrinsics__::AtomicIncrement(&m_RefCount);
            __intrinsics__::AtomicIncrement(&m_WeakRefCount);
      		return m_WeakRefCount;
		}

    	I32 Release() K3D_NOEXCEPT
		{
			assert((m_RefCount > 0) && (m_WeakRefCount > 0));
			if (__intrinsics__::AtomicDecrement(&m_RefCount) > 0)
			{
                __intrinsics__::AtomicDecrement(&m_WeakRefCount);
			}
			else
			{
				FreeValue();
				if (__intrinsics__::AtomicDecrement(&m_WeakRefCount) == 0)
				{
				I32 LastCount = m_WeakRefCount;
				FreeRefCountVal();
				return LastCount;
				}
			}
      		return m_WeakRefCount;
		}

		void AddWeakRef() K3D_NOEXCEPT { __intrinsics__::AtomicIncrement(&m_WeakRefCount); }

		void ReleaseWeakRef() K3D_NOEXCEPT
		{
			assert(m_WeakRefCount > 0);
			if (__intrinsics__::AtomicDecrement(&m_WeakRefCount) == 0)
				FreeRefCountVal();
		}

		RefCountBase* Lock() K3D_NOEXCEPT
		{
			for (I32 refCountTemp = m_RefCount; refCountTemp != 0; refCountTemp = m_RefCount)
			{
				if (__intrinsics__::AtomicCAS(&m_RefCount, refCountTemp + 1, refCountTemp))
				{
					__intrinsics__::AtomicIncrement(&m_WeakRefCount);
					return this;
				}
			}
			return nullptr;
		}

		virtual void FreeValue() K3D_NOEXCEPT = 0;
		virtual void FreeRefCountVal() K3D_NOEXCEPT = 0;
	};

	template <typename T, typename Deleter>
	class TRefCount : public RefCountBase
	{
	public:
		typedef TRefCount<T, Deleter>		this_type;
		typedef T                           value_type;
		typedef Deleter                     deleter_type;

		value_type     m_Value;
		deleter_type   m_Deleter;

		TRefCount(value_type value, deleter_type deleter)
			: RefCountBase(), m_Value(value), m_Deleter(Move(deleter))
		{}

		void FreeValue() K3D_NOEXCEPT override 
		{
			m_Deleter(m_Value);
			m_Value = nullptr;
		}

		void FreeRefCountVal() K3D_NOEXCEPT override 
		{
			this->~TRefCount();
			k3d_free(this, sizeof(*this));
		}

		void* GetDeleter() const
		{
			return (void*)&m_Deleter;
		}
	};


	template<typename T>
	class TRefCountInstance : public RefCountBase
	{
	public:
		typename AlignedStorage<sizeof(T), alignof(T)>::Type m_Memory; // use storage substitude

		T* GetValue() { return static_cast<T*>(static_cast<void*>(&m_Memory)); }

		template <typename... Args>
		TRefCountInstance(Args&&... args)
			: RefCountBase()
		{
			new (&m_Memory) T(Forward<Args>(args)...);
		}
		
		void FreeValue() K3D_NOEXCEPT override 
		{
			GetValue()->~T();
		}

		void FreeRefCountVal() K3D_NOEXCEPT override 
		{
			this->~TRefCountInstance();
			k3d_free(this, sizeof(*this));
		}
	};

}