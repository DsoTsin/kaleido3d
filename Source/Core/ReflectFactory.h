#pragma once

#include <KTL/Singleton.hpp>
#include <Interface/IReflectable.h>
#include <string>
#include <map>

namespace k3d 
{
	class ReflectFactory : public Singleton<ReflectFactory> {
	public:
		//template<typename IReflectable* T::(*CreateFunction)()>
		
		template <typename T>
		struct DelegateClassFunction {
			typedef T* (T::*Type)();
		};
		
		template <typename T>
		void Register(const char * className, typename DelegateClassFunction<T>::Type Reflect, T *obj) {
			//assert(obj);
			T* refElement = (obj->*Reflect)();
			m_ReflectMap[className] = refElement;
		}
		
		template <typename T, T* (*function)()>
		void Register(const char * className) {
			T* refElement = function();
			m_ReflectMap[className] = refElement;
		}

		void Register(const char * className, IReflectable * refElement)
		{
			//assert(className && refElement);
			m_ReflectMap[className] = refElement;
		}
		/*
		template <typename T>
		T * Get(const char *className)
		{
			return static_cast<T*>(m_ReflectMap[className]);
		}*/

		IReflectable * GetClass(const char *className) 
		{
			return m_ReflectMap[className];
		}
		
		typedef std::map<std::string, IReflectable*> ReflectMap;
	private:
		ReflectMap m_ReflectMap;
	};

}