#pragma once

namespace k3d
{
template<class T>
struct __RemoveRef
{
	typedef T Type;
};
template<class T>
struct __RemoveRef<T&>
{
	typedef T Type;
};
template<class T>
struct __RemoveRef<T&&>
{
	typedef T Type;
};
template<class T>
inline constexpr typename __RemoveRef<T>::Type&& Move(T&& arg)
{
	return static_cast<typename __RemoveRef<T>::Type&&>(arg);
}
template<class T>
inline constexpr T&& Forward(typename __RemoveRef<T>::Type& arg)
{
	return static_cast<T&&>(arg);
}

template<class T>
inline constexpr T&& Forward(typename __RemoveRef<T>::Type&& arg)
{
	//static_assert(!is_lvalue_reference<T>::value, "bad forward call");
	return (static_cast<T&&>(arg));
}

template<bool B, class T = void>
struct EnableIf {};

template<class T>
struct EnableIf<true, T> { typedef T Type; };

template <typename T> struct __AddRValueReference { typedef T&& type; };
template <typename T> struct __AddRValueReference<T&> { typedef T& type; };
template <>           struct __AddRValueReference<void> { typedef void type; };
template <>           struct __AddRValueReference<const void> { typedef const void type; };
template <>           struct __AddRValueReference<volatile void> { typedef volatile void type; };
template <>           struct __AddRValueReference<const volatile void> { typedef const volatile void type; };



template <typename T>
typename __AddRValueReference<T>::type Declval() K3D_NOEXCEPT;

// dynamic version
template <class T>
KFORCE_INLINE T Max(T const& L, T const& R)
{
    return L > R ? L : R;
}

template <class T>
KFORCE_INLINE T Min(T const& L, T const& R)
{
    return L < R ? L : R;
}

}