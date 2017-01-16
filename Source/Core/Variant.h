/**********************************************
Variant is for Streamable Class 
***********************************************/
#pragma once
#ifndef __k3dVariant_h__
#define __k3dVariant_h__
#include <Config/OSHeaders.h>

namespace k3d {
	/// Variant
	/// \brief The Variant class
	/// A class data Wrapper For all types
	class Variant
	{
	public:
		inline Variant() : m_DataSize(0), m_DataPtr(nullptr), m_isDeepCopied(false)
		{}

		~Variant() {
		}

		inline void Release() {
			if (m_DataSize != 0)
			{
				::free(m_DataPtr);
				m_DataSize = 0;
			}
		}

		inline void DeepCopy(void *data, int64 size, bool move = false)
		{
			if (m_DataPtr == nullptr)
			{
				m_DataPtr = malloc(size);
				m_DataSize = size;
			}
			::memcpy(m_DataPtr, data, size);
			if (move) {
				::free(data);
			}
			m_isDeepCopied = true;
		}

		inline Variant(void *data, int size)
		{
			m_DataPtr = data; m_DataSize = size;
			m_isDeepCopied = false;
		}

		const size_t  Size() const { return m_DataSize; }
		const void*   ConstData() const { return static_cast<const void*>(m_DataPtr); }
		void*         Data() { return m_DataPtr; }

		bool		IsDeepCopied() const { return m_isDeepCopied; }

		typedef const char Type;

	private:
		size_t  m_DataSize;
		void*   m_DataPtr;
		bool    m_isDeepCopied;
	};

	inline Archive & operator << (Archive & arch, Variant const & data) {
		arch.ArrayIn<Variant::Type>(static_cast<Variant::Type*>(data.ConstData()), (uint32)data.Size());
		return arch;
	}
}

#endif
