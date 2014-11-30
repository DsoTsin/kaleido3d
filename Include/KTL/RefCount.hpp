#pragma once

#include <Interface/IRefObj.h>

/**
* Wrapper for DirectX11 Objects
*/
template<typename RefObjType> 
class Ref
{
	typedef RefObjType* RefObjPtrType;
public:

	Ref(): m_RefPtr(nullptr) {}

	explicit Ref(RefObjType* refObj, bool bAddRef = true)
	{
		m_RefPtr = refObj;
		if(m_RefPtr && bAddRef)
		{
			m_RefPtr->AddRef();
		}
	}

	Ref(const Ref& rhs)
	{
		m_RefPtr = rhs.m_RefPtr;
		if(nullptr!=m_RefPtr)
		{
			m_RefPtr->AddRef();
		}
	}

	~Ref()
	{
		if(m_RefPtr)
		{
			m_RefPtr->Release();
		}
	}

	Ref& operator=(RefObjType* refObj)
	{
		RefObjType* OldReference = m_RefPtr;
		m_RefPtr = refObj;
		if(m_RefPtr)
		{
			m_RefPtr->AddRef();
		}
		if(OldReference)
		{
			OldReference->Release();
		}
		return *this;
	}

	Ref& operator=(const Ref& rhs)
	{
		return *this = rhs.m_RefPtr;
	}

	bool operator==(const Ref& rhs) const
	{
		return m_RefPtr == rhs.m_RefPtr;
	}

	RefObjType* operator->() const
	{
		return m_RefPtr;
	}

	operator RefObjPtrType() const
	{
		return m_RefPtr;
	}

	RefObjType** GetInitReference()
	{
		*this = nullptr;
		return &m_RefPtr;
	}

	RefObjType* GetReference() const
	{
		return m_RefPtr;
	}

	friend bool IsValidRef(const Ref& InReference)
	{
		return InReference.m_RefPtr != nullptr;
	}

	void SafeRelease()
	{
		*this = nullptr;
	}

	uint32 GetRefCount()
	{
		if(m_RefPtr)
		{
			m_RefPtr->AddRef();
			return m_RefPtr->Release();
		}
		else
		{
			return 0;
		}
	}

	void Swap(Ref& InPtr)
	{
		RefObjType* OldReference = m_RefPtr;
		m_RefPtr = InPtr.m_RefPtr;
		InPtr.m_RefPtr = OldReference;
	}
	
private:
	RefObjType* m_RefPtr;
};
